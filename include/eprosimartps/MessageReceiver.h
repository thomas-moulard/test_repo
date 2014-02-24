/*************************************************************************
 * Copyright (c) 2013 eProsima. All rights reserved.
 *
 * This copy of FastCdr is licensed to you under the terms described in the
 * EPROSIMARTPS_LIBRARY_LICENSE file included in this distribution.
 *
 *************************************************************************/

/*
 * MessageReceiver.h
 *
 *  Created on: Feb 20, 2014
 *      Author: Gonzalo Rodriguez Canosa
 *      email:  gonzalorodriguez@eprosima.com
 */



#ifndef MESSAGERECEIVER_H_
#define MESSAGERECEIVER_H_

#include "common/rtps_all.h"


namespace eprosima {
namespace rtps {

class MessageReceiver {
public:
	MessageReceiver();
	virtual ~MessageReceiver();
	void reset();
	/**
	 * Process a new CDR message.
	 * @param participantguidprefix Participant Guid Prefix
	 * @param address Address the message was received from
	 * @param data Pointer to data.
	 * @param length Length of the message in bytes.
	 */
	void processMsg(GuidPrefix_t participantguidprefix,
					octet*address,
			         const void*data, short length);



private:
	CDRMessage_t msg;
	ProtocolVersion_t sourceVersion;
	VendorId_t sourceVendorId;
	GuidPrefix_t sourceGuidPrefix;
	GuidPrefix_t destGuidPrefix;
	std::vector<Locator_t> unicastReplyLocatorList;
	std::vector<Locator_t> multicastReplyLocatorList;
	bool haveTimestamp;
	Time_t timestamp;

	ProtocolVersion_t destVersion;


	/**
	 * Extract header from data stream.
	 * @param msg Pointer to serialized Message
	 * @param H Pointer to header information.
	 * @return
	 */
	bool readHeader(CDRMessage_t* msg, Header_t*H);
	/**
	 * Read the submessage header.
	 * @param msg Pointer to the message.
	 * @param smh Pointer to the structure where the data should be saved.
	 * @return
	 */
	bool readSubmessageHeader(CDRMessage_t*msg, SubmessageHeader_t* smh);


	bool readSubmessageData(CDRMessage_t*msg, SubmessageHeader_t* smh,bool*last);
	bool readSubmessageHeartbeat(CDRMessage_t*msg, SubmessageHeader_t* smh,bool*last);
	bool readSubmessageGap(CDRMessage_t*msg, SubmessageHeader_t* smh,bool*last);
	bool readSubmessageAcknak(CDRMessage_t*msg, SubmessageHeader_t* smh,bool*last);
	bool readSubmessagePad(CDRMessage_t*msg, SubmessageHeader_t* smh,bool*last);
	bool readSubmessageInfoDestination(CDRMessage_t*msg, SubmessageHeader_t* smh,bool*last);
	bool readSubmessageInfoSource(CDRMessage_t*msg, SubmessageHeader_t* smh,bool*last);
	bool readSubmessageInfoTimestamp(CDRMessage_t*msg, SubmessageHeader_t* smh,bool*last);
	bool readSubmessageInfoReply(CDRMessage_t*msg, SubmessageHeader_t* smh,bool*last);




};

} /* namespace rtps */
} /* namespace eprosima */

#endif /* MESSAGERECEIVER_H_ */
