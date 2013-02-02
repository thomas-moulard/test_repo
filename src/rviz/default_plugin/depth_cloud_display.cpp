/*
 * Copyright (c) 2009, Willow Garage, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Willow Garage, Inc. nor the names of its
 *       contributors may be used to endorse or promote products derived from
 *       this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <QObject>

#include "depth_cloud_display.h"
#include "rviz/visualization_manager.h"
#include "rviz/properties/property.h"
#include "rviz/validate_floats.h"

#include <tf/transform_listener.h>

#include <boost/bind.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>

#include <OGRE/OgreSceneNode.h>
#include <OGRE/OgreSceneManager.h>

#include "image_transport/camera_common.h"
#include <image_transport/subscriber_plugin.h>

#include <sensor_msgs/image_encodings.h>

#include "depth_cloud_mld.h"

#include <sstream>
#include <string>
#include <math.h>

namespace enc = sensor_msgs::image_encodings;

namespace rviz
{

DepthCloudDisplay::DepthCloudDisplay()
  : rviz::Display()
  , messages_received_(0)
  , depthmap_it_(threaded_nh_)
  , depthmap_sub_()
  , rgb_it_ (threaded_nh_)
  , rgb_sub_()
  , cameraInfo_sub_()
  , queue_size_(5)
  , ml_depth_data_(new MultiLayerDepth())
  , angular_thres_(0.0f)
  , trans_thres_(0.0f)

{

  // Depth map properties
  QRegExp depth_filter("depth");
  depth_filter.setCaseSensitivity(Qt::CaseInsensitive);

  topic_filter_property_ = new Property("Topic Filter",
                                          true,
                                          "List only topics with names that relate to depth and color images",
                                          this,
                                          SLOT (updateTopicFilter() ));

  depth_topic_property_ = new RosFilteredTopicProperty("Depth Map Topic", "",
                                         QString::fromStdString(ros::message_traits::datatype<sensor_msgs::Image>()),
                                         "sensor_msgs::Image topic to subscribe to.", depth_filter, this, SLOT( updateTopic() ));

  depth_transport_property_ = new EnumProperty("Depth Map Transport Hint", "raw", "Preferred method of sending images.", this,
                                         SLOT( updateTopic() ));

  connect(depth_transport_property_, SIGNAL( requestOptions( EnumProperty* )), this,
          SLOT( fillTransportOptionList( EnumProperty* )));

  depth_transport_property_->setStdString("raw");

  // color image properties
  QRegExp color_filter("color|rgb|bgr|gray|mono");
  color_filter.setCaseSensitivity(Qt::CaseInsensitive);

  color_topic_property_ = new RosFilteredTopicProperty("Color Image Topic", "",
                                         QString::fromStdString(ros::message_traits::datatype<sensor_msgs::Image>()),
                                         "sensor_msgs::Image topic to subscribe to.", color_filter, this, SLOT( updateTopic() ));

  color_transport_property_ = new EnumProperty("Color Transport Hint", "raw", "Preferred method of sending images.", this,
                                         SLOT( updateTopic() ));


  connect(color_transport_property_, SIGNAL( requestOptions( EnumProperty* )), this,
          SLOT( fillTransportOptionList( EnumProperty* )));

  color_transport_property_->setStdString("raw");

  // Queue size property
  queue_size_property_ = new IntProperty( "Queue Size", queue_size_,
                                          "Advanced: set the size of the incoming message queue.  Increasing this "
                                          "is useful if your incoming TF data is delayed significantly from your"
                                          " image data, but it can greatly increase memory usage if the messages are big.",
                                          this, SLOT( updateQueueSize() ));
  queue_size_property_->setMin( 1 );

  use_auto_size_property_ = new BoolProperty( "Auto Size", true,
                                           "Automatically scale each point based on its depth value and the camera parameters.",
                                           this, SLOT( updateUseAutoSize() ), this );

  auto_size_factor_property_ = new FloatProperty( "Auto Size Factor", 1,
                                                "Scaling factor to be applied to the auto size.",
                                                this, SLOT( updateAutoSizeFactor() ), this );
  auto_size_factor_property_->setMin( 0.0001 );

  use_occlusion_compensation_property_ = new BoolProperty( "Occlusion Compensation", false,
          "Display occluded points within depth cloud",
          this, SLOT( updateUseOcclusionCompensation() ), this );

  occlusion_shadow_intensity_property_ = new FloatProperty( "Occlusion Shadow", 50.0f,
          "Control brightness of occluded points in percepnt",
          this, SLOT( updateOcclusionShadowIntensity() ), this );

  occlusion_shadow_timeout_property_ = new FloatProperty( "Occlusion Time-Out", 5.0f,
          "Amount of seconds before removing occluded points from the depth cloud",
          this, SLOT( updateOcclusionTimeOut() ), this );

  // Instantiate PointCloudCommon class for displaying point clouds
  pointcloud_common_ = new PointCloudCommon(this);

  updateUseAutoSize();

  // PointCloudCommon sets up a callback queue with a thread for each
  // instance.  Use that for processing incoming messages.
  threaded_nh_.setCallbackQueue( pointcloud_common_->getCallbackQueue() );

  // Scan for available transport plugins
  scanForTransportSubscriberPlugins();
}

void DepthCloudDisplay::onInitialize()
{
  pointcloud_common_->initialize(context_, scene_node_);
  pointcloud_common_->xyz_transformer_property_->hide();
}

DepthCloudDisplay::~DepthCloudDisplay()
{

  unsubscribe();

  if (pointcloud_common_)
    delete pointcloud_common_;

  if (ml_depth_data_)
	delete ml_depth_data_;
}

void DepthCloudDisplay::updateQueueSize()
{
  queue_size_ = queue_size_property_->getInt();
}

void DepthCloudDisplay::updateUseAutoSize()
{
  bool use_auto_size = use_auto_size_property_->getBool();
  pointcloud_common_->point_world_size_property_->setReadOnly( use_auto_size );
  pointcloud_common_->setAutoSize( use_auto_size );
  auto_size_factor_property_->setHidden(!use_auto_size);
}

void DepthCloudDisplay::updateAutoSizeFactor()
{
}

void DepthCloudDisplay::updateTopicFilter()
{
  bool enabled = topic_filter_property_->getValue().toBool();
  depth_topic_property_->enableFilter(enabled);
  color_topic_property_->enableFilter(enabled);
}

void DepthCloudDisplay::updateUseOcclusionCompensation()
{
  bool use_occlusion_compensation = use_occlusion_compensation_property_->getBool();
  occlusion_shadow_intensity_property_->setHidden(!use_occlusion_compensation);
  occlusion_shadow_timeout_property_->setHidden(!use_occlusion_compensation);
}
void DepthCloudDisplay::updateOcclusionShadowIntensity()
{

}
void DepthCloudDisplay::updateOcclusionTimeOut()
{
  bool use_occlusion_compensation = use_occlusion_compensation_property_->getBool();
}

void DepthCloudDisplay::onEnable()
{
  subscribe();
}

void DepthCloudDisplay::onDisable()
{
  unsubscribe();

  ml_depth_data_->reset();

  clear();
}

void DepthCloudDisplay::subscribe()
{
  if ( !isEnabled() )
  {
    return;
  }

  try
  {
    // reset all message filters
    sync_depth_color_.reset(new synchronizer_depth_color_(sync_policy_depth_color_(queue_size_)));
    depthmap_tf_filter_.reset();
    depthmap_sub_.reset(new image_transport::SubscriberFilter());
    rgb_sub_.reset(new image_transport::SubscriberFilter());
    cameraInfo_sub_.reset(new message_filters::Subscriber<sensor_msgs::CameraInfo>());

    std::string depthmap_topic = depth_topic_property_->getTopicStd();
    std::string color_topic = color_topic_property_->getTopicStd();

    std::string depthmap_transport = depth_transport_property_->getStdString();
    std::string color_transport = color_transport_property_->getStdString();

    if (!depthmap_topic.empty() && !depthmap_transport.empty()) {
      // subscribe to depth map topic
      depthmap_sub_->subscribe(depthmap_it_, depthmap_topic, queue_size_,  image_transport::TransportHints(depthmap_transport));

      depthmap_tf_filter_.reset(
          new tf::MessageFilter<sensor_msgs::Image>(*depthmap_sub_, *context_->getTFClient(), fixed_frame_.toStdString(), queue_size_, threaded_nh_));

      // subscribe to CameraInfo  topic
      std::string info_topic = image_transport::getCameraInfoTopic(depthmap_topic);
      cameraInfo_sub_->subscribe(threaded_nh_, info_topic, queue_size_);
      cameraInfo_sub_->registerCallback(boost::bind(&DepthCloudDisplay::caminfoCallback, this, _1));

      if (!color_topic.empty() && !color_transport.empty()) {
        // subscribe to color image topic
        rgb_sub_->subscribe(rgb_it_, color_topic, queue_size_,  image_transport::TransportHints(color_transport));

        // connect message filters to synchronizer
        sync_depth_color_->connectInput(*depthmap_tf_filter_, *rgb_sub_);
        sync_depth_color_->setInterMessageLowerBound(0, ros::Duration(0.5));
        sync_depth_color_->setInterMessageLowerBound(1, ros::Duration(0.5));
        sync_depth_color_->registerCallback(boost::bind(&DepthCloudDisplay::processMessage, this, _1, _2));

        pointcloud_common_->color_transformer_property_->setValue("RGB8");
      } else
      {
        depthmap_tf_filter_->registerCallback(boost::bind(&DepthCloudDisplay::processMessage, this, _1));
      }

    }
  }
  catch (ros::Exception& e)
  {
    setStatus( StatusProperty::Error, "Message", QString("Error subscribing: ") + e.what() );
  }
  catch (image_transport::TransportLoadException& e)
  {
    setStatus( StatusProperty::Error, "Message", QString("Error subscribing: ") + e.what() );
  }
}

void DepthCloudDisplay::caminfoCallback( const sensor_msgs::CameraInfo::ConstPtr& msg )
{
  boost::mutex::scoped_lock lock(camInfo_mutex_);
  camInfo_ = msg;
 }

void DepthCloudDisplay::unsubscribe()
{
  clear();

  try

  {
    // reset all filters
    sync_depth_color_.reset(new synchronizer_depth_color_(sync_policy_depth_color_(queue_size_)));
    depthmap_tf_filter_.reset();
    depthmap_sub_.reset();
    rgb_sub_.reset();
    cameraInfo_sub_.reset();
  }
  catch (ros::Exception& e)
  {
    setStatus( StatusProperty::Error, "Message", QString("Error unsubscribing: ") + e.what() );
  }
}


void DepthCloudDisplay::clear()
{

  boost::mutex::scoped_lock lock(mutex_);

  pointcloud_common_->reset();
}


void DepthCloudDisplay::update(float wall_dt, float ros_dt)
{

  boost::mutex::scoped_lock lock(mutex_);

  pointcloud_common_->update(wall_dt, ros_dt);
}


void DepthCloudDisplay::reset()
{
  clear();
  messages_received_ = 0;
  setStatus( StatusProperty::Ok, "Depth Map", "0 depth maps received" );
  setStatus( StatusProperty::Ok, "Message", "Ok" );
}

void DepthCloudDisplay::processMessage(const sensor_msgs::ImageConstPtr& depth_msg)
{

  processMessage(depth_msg, sensor_msgs::ImageConstPtr());

}

void DepthCloudDisplay::processMessage(const sensor_msgs::ImageConstPtr& depth_msg,
                                        const sensor_msgs::ImageConstPtr& rgb_msg)
{
   ++messages_received_;
   setStatus( StatusProperty::Ok, "Depth Map", QString::number(messages_received_) + " depth maps received");
   setStatus( StatusProperty::Ok, "Message", "Ok" );

   sensor_msgs::CameraInfo::ConstPtr camInfo;
   {
     boost::mutex::scoped_lock lock(camInfo_mutex_);
     camInfo = camInfo_;
   }

   if ( use_auto_size_property_->getBool() )
   {
     float f = camInfo->K[0];
     float s = auto_size_factor_property_->getFloat();
     pointcloud_common_->point_world_size_property_->setFloat( s / f );
   }

   Ogre::Quaternion orientation;
   Ogre::Vector3 position;

   if (!context_->getFrameManager()->getTransform(depth_msg->header, position,orientation))
   {
     setStatus(StatusProperty::Error, "Message",
    		 QString("Failed to transform from frame [")+depth_msg->header.frame_id.c_str()+
    		 QString("] to frame [")+context_->getFrameManager()->getFixedFrame().c_str()+
    		 QString("]") );
     return;
   } else
   {
	   Ogre::Radian angle;
	   Ogre::Vector3 axis;

	   (current_orientation_.Inverse() * orientation).ToAngleAxis(angle, axis);

	   float angle_deg = angle.valueDegrees();
	   if (angle_deg>=180.0f)
		   angle_deg -= 180.0f;
	   if (angle_deg<-180.0f)
		   angle_deg += 180.0f;

	   if (trans_thres_ == 0.0 || angular_thres_ == 0.0
	       || (position-current_position_).length() > trans_thres_
	       || angle_deg > angular_thres_)
	   {
	     current_position_ = position;
	     current_orientation_ = orientation;

	     ml_depth_data_->reset();
	   }

   }

   try
   {
	   ml_depth_data_->addDepthColorCameraInfo(depth_msg,rgb_msg,camInfo);
   }
   catch( MultiLayerDepthException& e )
   {
     setStatus( StatusProperty::Error, "Message", QString( "Error updating depth cloud: ") + e.what() );
   }

   // output pointcloud2 message
   sensor_msgs::PointCloud2Ptr cloud_msg = ml_depth_data_->generatePointCloud();

   cloud_msg->header = depth_msg->header;

   // add point cloud message to pointcloud_common to be visualized
   pointcloud_common_->addMessage(cloud_msg);

}


void DepthCloudDisplay::scanForTransportSubscriberPlugins()
{
  pluginlib::ClassLoader<image_transport::SubscriberPlugin> sub_loader("image_transport",
                                                                       "image_transport::SubscriberPlugin");

  BOOST_FOREACH( const std::string& lookup_name, sub_loader.getDeclaredClasses() )
  {
    // lookup_name is formatted as "pkg/transport_sub", for instance
    // "image_transport/compressed_sub" for the "compressed"
    // transport.  This code removes the "_sub" from the tail and
    // everything up to and including the "/" from the head, leaving
    // "compressed" (for example) in transport_name.
    std::string transport_name = boost::erase_last_copy(lookup_name, "_sub");
    transport_name = transport_name.substr(lookup_name.find('/') + 1);

    // If the plugin loads without throwing an exception, add its
    // transport name to the list of valid plugins, otherwise ignore
    // it.
    try
    {
      boost::shared_ptr<image_transport::SubscriberPlugin> sub = sub_loader.createInstance(lookup_name);
      transport_plugin_types_.insert(transport_name);
    }
    catch (const pluginlib::LibraryLoadException& e)
    {
    }
    catch (const pluginlib::CreateClassException& e)
    {
    }
  }
}

void DepthCloudDisplay::updateTopic()
{
  unsubscribe();
  reset();
  subscribe();
  context_->queueRender();
}

void DepthCloudDisplay::fillTransportOptionList(EnumProperty* property)
{
  property->clearOptions();

  std::vector<std::string> choices;

  choices.push_back("raw");

  // Loop over all current ROS topic names
  ros::master::V_TopicInfo topics;
  ros::master::getTopics(topics);
  ros::master::V_TopicInfo::iterator it = topics.begin();
  ros::master::V_TopicInfo::iterator end = topics.end();
  for (; it != end; ++it)
  {
    // If the beginning of this topic name is the same as topic_,
    // and the whole string is not the same,
    // and the next character is /
    // and there are no further slashes from there to the end,
    // then consider this a possible transport topic.
    const ros::master::TopicInfo& ti = *it;
    const std::string& topic_name = ti.name;
    const std::string& topic = depth_topic_property_->getStdString();

    if (topic_name.find(topic) == 0 && topic_name != topic && topic_name[topic.size()] == '/'
        && topic_name.find('/', topic.size() + 1) == std::string::npos)
    {
      std::string transport_type = topic_name.substr(topic.size() + 1);

      // If the transport type string found above is in the set of
      // supported transport type plugins, add it to the list.
      if (transport_plugin_types_.find(transport_type) != transport_plugin_types_.end())
      {
        choices.push_back(transport_type);
      }
    }
  }

  for (size_t i = 0; i < choices.size(); i++)
  {
    property->addOptionStd(choices[i]);
  }
}

void DepthCloudDisplay::fixedFrameChanged()
{
  Display::reset();
}

} // namespace rviz

#include <pluginlib/class_list_macros.h>

PLUGINLIB_EXPORT_CLASS( rviz::DepthCloudDisplay, rviz::Display)

