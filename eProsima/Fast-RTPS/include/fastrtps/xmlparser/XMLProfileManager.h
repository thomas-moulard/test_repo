// Copyright 2017 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#ifndef XML_PROFILE_MANAGER_H_
#define XML_PROFILE_MANAGER_H_

#include "stdio.h"
#include <string>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/xmlparser/XMLParserCommon.h>
#include <fastrtps/xmlparser/XMLParser.h>
#include <map>



namespace eprosima{
namespace fastrtps{
namespace xmlparser{

typedef std::map<std::string, up_participant_t> participant_map_t;
typedef participant_map_t::iterator             part_map_iterator_t;
typedef std::map<std::string, up_publisher_t>   publisher_map_t;
typedef publisher_map_t::iterator               publ_map_iterator_t;
typedef std::map<std::string, up_subscriber_t>  subscriber_map_t;
typedef subscriber_map_t::iterator              subs_map_iterator_t;
typedef std::map<std::string, XMLP_ret>         xmlfiles_map_t;
typedef xmlfiles_map_t::iterator                xmlfile_map_iterator_t;


/**
 * Class XMLProfileManager, used to make available profiles from XML file.
 * @ingroup XMLPROFILEMANAGER_MODULE
 */
class XMLProfileManager
{

public:
    /**
    * Load the default profiles XML file.
    * @return XMLP_ret::XML_OK on success, XMLP_ret::XML_ERROR in other case.
    */
    RTPS_DllAPI static XMLP_ret loadDefaultXMLFile();

    /**
    * Load a profiles XML file.
    * @param filename Name for the file to be loaded.
    * @return XMLP_ret::XML_OK on success, XMLP_ret::XML_ERROR in other case.
    */
    RTPS_DllAPI static XMLP_ret loadXMLFile(const std::string &filename);

    /**
    * Search for the profile specified and fill the structure.
    * @param profile_name Name for the profile to be used to fill the structure.
    * @param atts Structure to be filled.
    * @return XMLP_ret::XML_OK on success, XMLP_ret::XML_ERROR in other case.
    */
    RTPS_DllAPI static XMLP_ret fillParticipantAttributes(const std::string &profile_name, ParticipantAttributes &atts);

    RTPS_DllAPI static void getDefaultParticipantAttributes(ParticipantAttributes& participant_attributes);

    /**
    * Search for the profile specified and fill the structure.
    * @param profile_name Name for the profile to be used to fill the structure.
    * @param atts Structure to be filled.
    * @return XMLP_ret::XML_OK on success, XMLP_ret::XML_ERROR in other case.
    */
    RTPS_DllAPI static XMLP_ret fillPublisherAttributes(const std::string &profile_name, PublisherAttributes &atts);

    RTPS_DllAPI static void getDefaultPublisherAttributes(PublisherAttributes& publisher_attributes);

    /**
    * Search for the profile specified and fill the structure.
    * @param profile_name Name for the profile to be used to fill the structure.
    * @param atts Structure to be filled.
    * @return XMLP_ret::XML_OK on success, XMLP_ret::XML_ERROR in other case.
    */
    RTPS_DllAPI static XMLP_ret fillSubscriberAttributes(const std::string &profile_name, SubscriberAttributes &atts);

    RTPS_DllAPI static void getDefaultSubscriberAttributes(SubscriberAttributes& subscriber_attributes);

private:
    
    RTPS_DllAPI static XMLP_ret extractProfiles(up_base_node_t properties, const std::string& filename);
    RTPS_DllAPI static XMLP_ret extractParticipantProfile(up_base_node_t& profile, const std::string& filename);
    RTPS_DllAPI static XMLP_ret extractPublisherProfile(up_base_node_t& profile, const std::string& filename);
    RTPS_DllAPI static XMLP_ret extractSubscriberProfile(up_base_node_t& profile, const std::string& filename);

    static BaseNode* root;
    static participant_map_t m_participant_profiles;
    static publisher_map_t   m_publisher_profiles;
    static subscriber_map_t  m_subscriber_profiles;
    static xmlfiles_map_t    m_xml_files;
};

} /* xmlparser */
} /* namespace */
} /* namespace eprosima */

#endif
