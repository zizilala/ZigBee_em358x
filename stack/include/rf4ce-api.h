/**
 * @file rf4ce-api.h
 * @brief ZigBee RF4CE stack APIs and callbacks.
 * See @ref rf4ce for documentation.
 *
 * <!--Copyright 2013 Silicon Laboratories, Inc.                         *80*-->
 */

#ifndef __RF4CE_API_H__
#define __RF4CE_API_H__

/** @brief Sets the pairing table entry corresponding to the passed index.
 *
 * @param pairingIndex   The index of the pairing table entry to be set.
 *
 * @param entry    A pointer to an ::EmberRf4cePairingTableEntry struct to be
 *                 copied into the pairing table at the passed index. If the
 *                 passed pointer is NULL, the stack will delete the entry
 *                 stored at the passed pairing index.
 *
 * @return   An ::EmberStatus value of ::EMBER_SUCCESS if the pairing table
 * entry was successfully set. An ::EmberStatus value of ::EMBER_INVALID_CALL if
 * the current network is not an RF4CE network, or the node hasn't been started,
 * or the node is currently busy performing some discovery or pairing process.
 * An ::EmberStatus value of ::EMBER_INDEX_OUT_OF_RANGE if the specified index
 * is outside the valid range.
 */
EmberStatus emberRf4ceSetPairingTableEntry(int8u pairingIndex,
                                           EmberRf4cePairingTableEntry *entry);

/** @brief Retrieves the pairing table entry stored at the passed index.
 *
 * @param pairingIndex   The index of the requested pairing table entry.
 *
 * @param entry    A pointer to an ::EmberRf4cePairingTableEntry struct where
 *  the requested pairing table entry will be copied.
 *
 * @return   An ::EmberStatus value of ::EMBER_SUCCESS if the requested entry
 * was successfully copied in the passed ::EmberRf4cePairingTableEntry struct.
 * An ::EmberStatus value of ::EMBER_INVALID_CALL if the current network is not
 * an RF4CE network, or the node hasn't been started or the node is currently
 * busy performing some discovery or pairing process. An ::EmberStatus value of
 * ::EMBER_INDEX_OUT_OF_RANGE if the specified index is outside the valid range.
 */
EmberStatus emberRf4ceGetPairingTableEntry(int8u pairingIndex,
                                           EmberRf4cePairingTableEntry *entry);

/** @brief Sets the application information of the node.
 *
 * @param appInfo   A pointer to an ::EmberRf4ceApplicationInfo containing the
 * application information to be set at the node.
 *
 * @return   An ::EmberStatus value of ::EMBER_SUCCESS if the application
 * information was successfully set. An ::EmberStatus value of
 * ::EMBER_INVALID_CALL if the stack failed to set the application information.
 * For instance, the application information can not be changed if the stack
 * is currently involved in a discovery or pairing process.
 */
EmberStatus emberRf4ceSetApplicationInfo(EmberRf4ceApplicationInfo *appInfo);

/** @brief Retrieves the application information of the node.
 *
 * @param appInfo   A pointer to an ::EmberRf4ceApplicationInfo where the stack
 * will copy the application information of the node.
 *
 * @return   An ::EmberStatus value of ::EMBER_SUCCESS if the application
 * information was successfully retrieved. An ::EmberStatus value of
 * ::EMBER_INVALID_CALL if the stack failed to retrieve the application
 * information.
 */
EmberStatus emberRf4ceGetApplicationInfo(EmberRf4ceApplicationInfo *appInfo);

/** @brief An API for updating the link key of a pairing table entry.
 *
 * @param pairingIndex     The index of the pairing table entry to be updated.
 *
 * @param key              A pointer to an ::EmberKeyData struct containing the
 *                         new key.
 *
 * @return  An ::EmberStatus value of EMBER_SUCCESS if the pairing table entry
 * was successfully updated. An ::EmberStatus value of EMBER_INDEX_OUT_OF_RANGE
 * if the specified index is outside the valid range. An ::EmberStatus value of
 * EMBER_INVALID_CALL if the current network is not an RF4CE network, or the
 * node hasn't been started, or the node is currently busy performing some
 * discovery or pairing process, or if the passed pairing index does not
 * correspond to an active secured pairing link and/or the node does not support
 * security.
 */
EmberStatus emberRf4ceKeyUpdate(int8u pairingIndex, EmberKeyData *key);

/** @brief Sends a message as per the ZigBee RF4CE specification.
 *
 * @param pairingIndex    The index of the entry in the pairing table to be used
 *  to transmit the packet. this parameter is ignored if broadcast bit is set in
 *  the txOptions bitmask.
 *
 * @param profileId       The profile ID to be included in the RF4CE network
 *  header of the outgoing RF4CE network DATA frame.
 *
 * @param vendorId        The vendor ID to be included in the RF4CE network
 *  header of the outgoing RF4CE network DATA frame. This field is meaningful
 *  only if the EMBER_RF4CE_TX_OPTIONS_VENDOR_SPECIFIC_BIT is set in the
 *  txOptions bitmask.
 *
 * @param txOptions       7-bit transmission options bitmask as per ZigBee RF4CE
 *  specification (see Table 2, section 3.1.1.1.1).
 *
 * @param messageLength   The length in bytes of the message to be sent.
 *
 * @param message         A pointer to the message to be sent.
 *
 * @return   An ::EmberStatus value. For any result other than ::EMBER_SUCCESS,
 * the message will not be sent.
 * - ::EMBER_SUCCESS - The message has been submitted for transmission.
 * - ::EMBER_INDEX_OUT_OF_RANGE - If the application requested a unicast
 *  transmission and the passed pairingIndex parameter is an invalid index.
 * - ::EMBER_INVALID_CALL - If at least one of the following requirements are
 *  not met.
 *    - The current network is an RF4CE network.
 *    - The current network is up and running.
 *    - The node is busy performing some discovery or pairing process.
 *    - Unicast transmissions require that the passed pairingIndex corresponds
 *      to an active pairing entry.
 *    - Broadcast transmissions can not be acked, encrypted or use long
 *      addressing.
 *    - Secured transmissions can only be unicast and require a secured pairing
 *      entry.
 *    - If channel designator is to be included in the packet, the transmission
 *      must be acked, the node must be a target and the destination must
 *      support channel normalization.
 * - ::EMBER_NO_BUFFERS - The message was not sent because of RAM shortage.
 * - ::EMBER_DELIVERY_FAILED - The message was rejected by the Network or MAC
 *  layers.
 */
EmberStatus emberRf4ceSend(int8u pairingIndex,
                           int8u profileId,
                           int16u vendorId,
                           EmberRf4ceTxOption txOptions,
                           int8u messageLength,
                           int8u *message);

/** @brief A callback invoked by the ZigBee RF4CE stack when it has completed
 * sending a message.
 *
 *  @param status         An ::EmberStatus value of:
 *  - ::EMBER_SUCCESS - The message was successfully delivered.
 *  - ::EMBER_DELIVERY_FAILED - The message was not delivered.
 *
 * @param pairingIndex    The index of the entry in the pairing table used to
 *  transmit the message.
 *
 * @param txOptions       The TX options bitmask as per ZigBee RF4CE
 *                        specification used for transmitting the packet.
 *
 * @param profileId       The profile ID included in the message.
 *
 * @param vendorId        The vendor ID included in the message, if any.
 *
 * @param messageLength   The length in bytes of the message.
 *
 * @param message         A pointer to the payload of the message that was sent.
 */
void emberRf4ceMessageSentHandler(EmberStatus status,
                                  int8u pairingIndex,
                                  int8u txOptions,
                                  int8u profileId,
                                  int16u vendorId,
                                  int8u messageLength,
                                  int8u *message);

/** @brief A callback invoked by the ZigBee RF4CE stack when a message is
 * received.
 *
 * @param pairingIndex    The index of the entry in the pairing table
 * corresponding to the PAN on which the message was received.
 *
 * @param profileId       The profile ID included in the message.
 *
 * @param vendorId        The vendor ID included in the message, if any.
 *
 * @param messageLength   The length in bytes of the received message.
 *
 * @param message         A pointer to the payload of the received message.
 */
void emberRf4ceIncomingMessageHandler(int8u pairingIndex,
                                      int8u profileId,
                                      int16u vendorId,
                                      int8u messageLength,
                                      int8u *message);

/** @brief The node starts the network operations. If the node is started as
 * target, it will attempt to form a new RF4CE PAN by performing an energy
 * scan for determining the best channel, by performing an active scan to
 * determine an unique pan ID and unique network address. Once all these steps
 * are successfully completed, the node starts normal network operations as
 * RF4CE target. If the node is started as controller, no scanning will be
 * performed.
 *
 * @param capabilities    An ::EmberRf4ceNodeCapabilities that specifies the
 *                        node capabilities as described in section 3.4.2.4.
 *
 * @param vendorInfo      A pointer to an ::EmberRf4ceVendorInfo struct
 *                        containing the vendor information of the node.
 *
 * @param appInfo         A pointer to an ::EmberRf4ceApplicationInfo struct
 *                        containing the application information of the node.
 *
 * @param power           The radio power the node should use in transmission.
 *
 * @return   An ::EmberStatus value of ::EMBER_SUCCESS if the starting network
 * operations are successfully initiated, ::EMBER_INVALID_CALL if one or more
 * passed parameters are invalid, or the current network is already up, or the
 * node is already up as "always on" node on another network (an "always on"
 * node is a non-sleepy ZigBee PRO node or a ZigBee RF4CE node).
 */
EmberStatus emberRf4ceStart(EmberRf4ceNodeCapabilities capabilities,
                            EmberRf4ceVendorInfo *vendorInfo,
                            EmberRf4ceApplicationInfo *appInfo,
                            int8s power);

/* @brief The node stops the network operations and clears the network state
 * and the pairing table stored in persistent memory.
 *
 * @return   An ::EmberStatus value of ::EMBER_SUCCESS if the network operations
 * are successfully terminated, ::EMBER_INVALID_CALL if the current network is
 * not an RF4CE network or the current network is not up or the stack is
 * currently handling some discovery or pairing process.
 */
EmberStatus emberRf4ceStop(void);

/** @brief The node performs a discovery of ZigBee RF4CE nodes that matches the
 *  requirements specified in the passed parameters.
 *
 * @param panId     The PAN ID of the destination device for the discovery. This
 *                  value can be set to EMBER_RF4CE_BROADCAST_PAN_ID to indicate
 *                  a wildcard.
 *
 * @param nodeId    The network address of the destination device for the
 *                  discovery. This value can be set to
 *                  EMBER_RF4CE_BROADCAST_ADDRESS to indicate a wildcard.
 *
 * @param searchDevType  The device type to discover. This value can be set to
 *                       0xFF to indicate a wildcard.
 *
 * @param discDuration      The time (in milliseconds) to wait for discovery
 *                          responses to be sent back from potential target
 *                          nodes on each channel.
 *
 * @param maxDiscRepetitions   The maximum number of discovery trials. A
 *                             discovery trial is defined as the transmission of
 *                             a discovery request command frame on all
 *                             available channels.
 *
 * @param discProfileIdListLength  The length of the discovery profile ID list.
 *                                 The stack supports up to 7 profile ID
 *                                 entries.
 *
 * @param discProfileIdList The list of profile IDs against which profile IDs
 *                          contained in received discovery response command
 *                          frames will be matched for acceptance.
 *
 * @return   An ::EmberStatus value of ::EMBER_SUCCESS if the discovery process
 * started successfully, ::EMBER_INVALID_CALL if the node is down (either the
 * node was cold started with the ::emberRf4ceStart() or warm started with
 * ::emberRf4ceNetworkInit()), or the current network is not an RF4CE network,
 * or the node is already in the middle of a discovery or pairing process, or
 * another ::EmberStatus value indicating the error occurred.
 */
EmberStatus emberRf4ceDiscovery(EmberPanId panId,
                                EmberNodeId nodeId,
                                int8u searchDevType,
                                int16u discDuration,
                                int8u maxDiscRepetitions,
                                int8u discProfileIdListLength,
                                int8u *discProfileIdList);

/** @brief A callback invoked by the ZigBee RF4CE stack when it has completed
 *  the discovery process.
 *
 * @param status  An ::EmberStatus value of ::EMBER_SUCCESS if discovery has
 *                been correctly performed over the three RF4CE channels and at
 *                least a valid discovery response was received.
 *                An ::EmberStatus value of ::EMBER_DISCOVERY_TIMEOUT if the
 *                discovery process completed and no valid discovery response
 *                was received.
 *                Otherwise, another ::EmberStatus value indicating the error
 *                occurred.
 */
void emberRf4ceDiscoveryCompleteHandler(EmberStatus status);

/** @brief A callback invoked by the ZigBee RF4CE stack when a discovery request
 * is received. If the callback returns TRUE, the stack shall respond with a
 * discovery response, otherwise it will silently discard the discovery request
 * message.
 *
 * @param srcIeeeAddr       The IEEE address of the node that issued the
 *                          discovery request.
 *
 * @param nodeCapabilities  The node capabilities of the node that issued the
 *                          discovery request.
 *
 * @param vendorInfo        A pointer to an ::EmberRf4ceVendorInfo struct
 *                          containing the vendor information of the node that
 *                          issued the discovery request.
 *
 * @param appInfo           A pointer to an ::EmberRf4ceApplicationInfo struct
 *                          containing the application information of the node
 *                          that issued the discovery request.
 *
 * @param searchDevType     The device type being discovered. If this is 0xFF,
 *                          any type is being requested.
 *
 * @param rxLinkQuality     LQI value, as passed via the MAC sub-layer, of the
 *                          discovery request command frame.
 *
 * @return   FALSE if the discovery request should be discarded. Return TRUE if
 * the application wants to respond to the discovery request.
 */
boolean emberRf4ceDiscoveryRequestHandler(EmberEUI64 srcIeeeAddr,
                                          int8u nodeCapabilities,
                                          EmberRf4ceVendorInfo *vendorInfo,
                                          EmberRf4ceApplicationInfo *appInfo,
                                          int8u searchDevType,
                                          int8u rxLinkQuality);

/** @brief A callback invoked by the ZigBee RF4CE stack when a discovery
 * response is received.
 *
 * @param atCapacity        A boolean set to TRUE if the node sending the
 *                          discovery response has no free entry in its pairing
 *                          table, FALSE otherwise.
 *
 * @param channel           The channel on which the discovery response was
 *                          received.
 *
 * @param panId             The PAN identifier of the responding device.
 *
 * @param srcIeeeAddr       The IEEE address of the responding device.
 *
 * @param nodeCapabilities  The capabilities of the responding node.
 *
 * @param vendorInfo        The vendor information of the responding device.
 *
 * @param appInfo           The application information of the responding
 *                          device.
 *
 * @param rxLinkQuality     LQI value, as passed via the MAC sub-layer, of the
 *                          discovery response command frame.
 *
 * @param discRequestLqi    The LQI of the discovery request command frame
 *                          reported by the responding device.
 *
 * @return   If this callback returns TRUE the stack will continue the discovery
 * process. If this callback returns FALSE, the discovery process will end at
 * the end of the current discovery trial. A discovery trial is defined as the
 * transmission of a discovery request command frame on all available channels.
 */
boolean emberRf4ceDiscoveryResponseHandler(boolean atCapacity,
                                           int8u channel,
                                           EmberPanId panId,
                                           EmberEUI64 srcIeeeAddr,
                                           int8u nodeCapabilities,
                                           EmberRf4ceVendorInfo *vendorInfo,
                                           EmberRf4ceApplicationInfo *appInfo,
                                           int8u rxLinkQuality,
                                           int8u discRequestLqi);

/** @brief The node automatically handles the receipt of discovery request
 * command frames. Note that during this auto discovery response mode, the stack
 * does not inform the application of the arrival of discovery request command
 * frames. Furthermore, during this mode, if the node receives a command frame
 * that is not a discovery request command frame, it will be discarded. At the
 * end of the auto discovery response mode, the stack will call the
 * ::emberRf4ceAutoDiscoveryResponseCompleteHandler() callback.
 *
 * @param duration    The maximum duration, in milliseconds, while the node
 *                    will be in auto discovery response mode.
 *
 * @return   An ::EmberStatus value of EMBER_SUCCESS if the node successfully
 * entered in auto discovery response mode. An ::EmberStatus value of
 * EMBER_INVALID_CALL if the node has not been started, or the current network
 * is not an RF4CE network, or the node is already in the middle of another
 * discovery or pairing process.
 */
EmberStatus emberRf4ceEnableAutoDiscoveryResponse(int16u duration);

/** @brief A callback invoked by the ZigBee RF4CE stack when it has completed
 * the requested auto discovery response phase.
 *
 * @param status       An ::EmberStatus value of EMBER_SUCCESS indicating that
 *                     it successfully received a discovery request frame twice
 *                     from the same node with IEEE address specified by the
 *                     scrIeeeAddr parameter. An ::EmberStatus value of
 *                     EMBER_DISCOVERY_TIMEOUT if the node has not received the
 *                     two discovery request frame within the auto discovery
 *                     response duration interval. An ::EmberStatus value of
 *                     EMBER_DISCOVERY_ERROR if the node has received two
 *                     valid discovery request command frames from two different
 *                     nodes. An ::EmberStatus value of EMBER_NO_BUFFERS if the
 *                     node could not respond because of RAM shortage. An
 *                     ::EmberStatus value of EMBER_ERR_FATAL if the MAC layer
 *                     rejected the discovery response.
 *
 * @param srcIeeeAddr  An ::EmberEUI64 value indicating the IEEE address from
 *                     which the discovery request command frame was received.
 *                     This parameter is non-NULL only if the status parameter
 *                     is EMBER_SUCCESS.
 *
 * @param nodeCapabilities  The node capabilities of the node that issued the
 *                          discovery request. This parameter is meaningful
 *                          only if the status parameter is EMBER_SUCCESS.
 *
 * @param vendorInfo        A pointer to an ::EmberRf4ceVendorInfo struct
 *                          containing the vendor information of the node that
 *                          issued the discovery request. This parameter is
 *                          non-NULL only if the status parameter is
 *                          EMBER_SUCCESS.
 *
 *
 * @param appInfo           A pointer to an ::EmberRf4ceApplicationInfo struct
 *                          containing the application information of the node
 *                          that issued the discovery request. This parameter is
 *                          non-NULL only if the status parameter is
 *                          EMBER_SUCCESS.
 *
 * @param searchDevType     The device type being discovered. If this is 0xFF,
 *                          any type is being requested. This parameter is
 *                          meaningful only if the status parameter is
 *                          EMBER_SUCCESS.
 */
void emberRf4ceAutoDiscoveryResponseCompleteHandler(EmberStatus status,
                                                    EmberEUI64 srcIeeeAddr,
                                                    int8u nodeCapabilities,
                                                    EmberRf4ceVendorInfo *vendorInfo,
                                                    EmberRf4ceApplicationInfo *appInfo,
                                                    int8u searchDevType);

/** @brief The node initiates the Rf4CE pairing process according to the
 * specified parameters.
 *
 * @param channel     The logical channel of the device with which to pair.
 *
 * @param panId       The PAN identifier of the device with which to pair.
 *
 * @param ieeeAddr    The IEEE address of the device with which to pair.
 *
 * @param keyExchangeTransferCount  The number of transfers the target should use
 *                                 to exchange the link key with the pairing
 *                                 originator.
 *
 * @return   An ::EmberStatus value of EMBER_SUCCESS if the pairing process was
 * successfully initiated and an unused pairing entry will be created for the
 * new pairing. An ::EmberStatus value of EMBER_DUPLICATE_ENTRY if the pairing
 * process was successfully initiated and an existing pairing entry will be
 * updated. An ::EmberStatus value of EMBER_INVALID_CALL if the node has not
 * been started, or the current network is not an RF4CE network, or the node is
 * already in the middle of another discovery or pairing process.
 * Another ::EmberStatus value indicating the specific error occurred otherwise.
 */
EmberStatus emberRf4cePair(int8u channel,
                           EmberPanId panId,
                           EmberEUI64 ieeeAddr,
                           int8u keyExchangeTransferCount);

/** @brief A callback invoked by the ZigBee RF4CE stack when the originator or
 * the recipient node has completed the pairing process.
 *
 * @param status        An ::EmberStatus value of EMBER_SUCCESS if the pairing
 *                      process succeeded and a pairing link has been
 *                      established. An ::EmberStatus value of
 *                      EMBER_NO_RESPONSE if the originator has timed out
 *                      waiting for the pair response or for the ping response
 *                      during the link key exchange procedure. An ::EmberStatus
 *                      value of EMBER_TABLE_FULL if a pair response was
 *                      received at the originator indicating that the recipient
 *                      device has no available entry in its pairing table.
 *                      An ::EmberStatus value of EMBER_NOT_PERMITTED if a pair
 *                      response was received at the originator indicating that
 *                      the recipient device did not accept the pair request.
 *                      An ::EmberStatus value of EMBER_SECURITY_TIMEOUT if the
 *                      node has timed out during the link key exchange or
 *                      recovery procedures. An ::EmberStatus value of
 *                      EMBER_SECURITY_FAILURE if some other error occurred
 *                      during the link key exchange or recovery procedures.
 *                      Another ::EmberStatus value indicating the specific
 *                      reason why the originator or the recipient node failed
 *                      to deliver a command frame.
 * @param pairingIndex  The index of the pairing table entry corresponding
 *                      to the pairing link that was established during the
 *                      pairing process. This field is meaningful only if the
 *                      status parameter is EMBER_SUCCESS.
 */
void emberRf4cePairCompleteHandler(EmberStatus status,
                                   int8u pairingIndex);

/** @brief A callback invoked by the ZigBee RF4CE stack when a pair request has
 * been received.
 *
 * @param status        An ::EmberStatus value of EMBER_SUCCESS if the request
 *                      pairing is not a duplicate pairing and an unused entry
 *                      in the pairing table is available. An ::EmberStatus
 *                      value of EMBER_TABLE_FULL if the request pairing is not
 *                      a duplicate pairing and the pairing table is full.
 *                      An ::EmberStatus value of EMBER_DUPLICATE_ENTRY if the
 *                      request pairing is a duplicate pairing. In this case,
 *                      the stack will update the entry indicated by the
 *                      pairingIndex parameter.
 *
 * @param pairingIndex  The index of the entry that will be used by the stack
 *                      for the pairing link. If the status parameter is
 *                      EMBER_TABLE_FULL this parameter is meaningless.
 *
 * @param srcIeeeAddr   An ::EmberEUI64 value indicating the source IEEE
 *                      address of the incoming pair request command.
 *
 * @param nodeCapabilities  The node capabilities of requesting device.
 *
 * @param vendorInfo    The vendor information of the requesting device.
 *
 * @param appInfo       The application information of the requesting device.
 *
 * @param keyExchangeTransferCount  The number of transfers to be used to
 *                                  exchange the link key with the pairing
 *                                  originator, indicated in the incoming pair
 *                                  request command.
 *
 * @return   TRUE if the application accepts the pair, FALSE otherwise.
 */
boolean emberRf4cePairRequestHandler(EmberStatus status,
                                     int8u pairingIndex,
                                     EmberEUI64 srcIeeeAddr,
                                     int8u nodeCapabilities,
                                     EmberRf4ceVendorInfo *vendorInfo,
                                     EmberRf4ceApplicationInfo *appInfo,
                                     int8u keyExchangeTransferCount);

/** @brief The node attempts to remove the specified pairing link.
 * If successful, the node transmit an unpair request command frame to the peer
 * node. The pairing link will be then removed regardless on whether the peer
 * node has acknowledged the unpair request command frame.
 *
 * @param pairingIndex     The index of the pairing link to be removed.
 *
 * @return   An ::EmberStatus value of EMBER_SUCCESS if the unpairing process
 * has successfully started. An ::EmberStatus value of EMBER_INVALID_CALL if the
 * node has not been started, or the current network is not an RF4CE network, or
 * if the node is in the middle of a discovery or pairing process or if the
 * passed index does not correspond to an active pairing link. An ::EmberStatus
 * value of EMBER_NO_BUFFERS if the unpair request command frame was not sent
 * because of RAM shortage. An ::EmberStatus value of EMBER_ERR_FATAL if the MAC
 * layer rejected the unpair request command frame.
 */
EmberStatus emberRf4ceUnpair(int8u pairingIndex);

/** @brief A callback invoked by the ZigBee RF4CE stack when an unpair command
 * frame has been received. The stack will remove the pairing link indicated by
 * the passed index.
 *
 * @param pairingIndex     The index of the pairing link to be removed.
 *
 * @return   An ::EmberStatus value of EMBER_SUCCESS if the unpairing process
 * was successfully initiated. Another ::EmberStatus value indicating the
 * specific error occurred otherwise.
 */
void emberRf4ceUnpairHandler(int8u pairingIndex);

/** @brief A callback invoked by the ZigBee RF4CE stack when the unpair
 * procedure has been completed. According to the RF4CE specs, during the unpair
 * procedure, the stack sends an unpair command frame. If the command is not
 * successfully delivered, the stack tries another RF4CE channel until the frame
 * is received or the stack already tried all the RF4CE channels. Either way, at
 * the end of the unpair process the pairing table entry is deleted and this
 * callback is invoked.
 *
 * @param pairingIndex     The index of the removed pairing link.
 */
void emberRf4ceUnpairCompleteHandler(int8u pairingIndex);

/** @brief  The node enables or disables RF4CE power saving mode according to
 * the passed parameters. If dutyCycle is 0x000000, power saving mode is
 * disabled and the node radio will be always on listening for incoming packets.
 * If activePeriod is 0x000000 and dutyCycle is > 0 the radio is turned off
 * until further notice.
 * Otherwise, the stack will enable power saving mode and switch the radio on
 * and off according to the passed parameters, whereas the node's radio will be
 * on for activePeriod milliseconds every dutyCycle milliseconds.
 *
 * @param dutyCycle     The duty cycle of a device in milliseconds. A value of
 *                      0x000000 disable power saving management at the RF4CE
 *                      stack and the application must provide this
 *                      functionality directly. Legal values for this parameter
 *                      0x000000 and any value in the range
 *                      [nwkcMinActivePeriod=16.8ms, nwkcMaxDutyCycle=1s].
 *
 * @param activePeriod  The active period of a device in milliseconds. Legal
 *                      values for this parameter fall in the set
 *                      [nwkcMinActivePeriod=16.8ms, dutyCycle) U {0x000000}.
 *                      If the dutyCycle parameter is 0, the activePeriod
 *                      parameter is ignored.
 *
 * @return   An ::EmberStatus value of EMBER_SUCCESS if the power saving
 * parameters were correctly set. An ::EmberStatus value of EMBER_INVALID_CALL
 * if the node has not been started yet, or the current network is not an RF4CE
 * network, or the node is currently busy with some discovery or pairing
 * process, or if any of the passed parameters is not in the legal range.
 */
EmberStatus emberRf4ceSetPowerSavingParameters(int32u dutyCycle,
                                               int32u activePeriod);

/** @brief  Every target node periodically performs an energy scan of the base
 * channel to detect whether the channel becomes compromised. In particular if
 * channelChangeReads RSSI reads out of the last rssiWindowSize RSSI reads are
 * above the rssiThreshold, the target will switch its base channel to the next
 * channel.
 *
 * @param rssiWindowSize  Defines the size of the RSSI reads window, that is,
 *                        the number of the most recent RSSI reads that are
 *                        taken into consideration to decide whether a channel
 *                        switch is required or not. Valid values for this
 *                        parameter fall in the interval [1,32]. Setting this
 *                        parameter to 0 disables frequency agility at the
 *                        target. This parameter is set by default to 16.
 *
 * @params channelChangeReads   Defines the number of RSSI reads above the RSSI
 *                              threshold that will trigger a channel switch.
 *                              Valid values for this parameter fall in the
 *                              interval [1,rssiWindowSize]. This parameter is
 *                              set by default to 15.
 *
  * @param rssiThreshold  Defines the RSSI threshold. A RSSI value resulting
 *                        from the periodic scan that is above this threshold is
 *                        considered a 'channel congested' read. If the stack
 *                        detects multiple channel congested reads, it will
 *                        eventually move the base channel to the next RF4CE
 *                        channel. This parameter is set by default to the
 *                        CCA threshold.
 *
 * @param  readInterval   The interval length (in seconds) between two
 *                        consecutive RSSI reads. This parameter is set by
 *                        default to 10 seconds.
 *
 * @param readDuration    Sets the exponent of the number of scan periods of the
 *                        RSSI read process, where a scan period is 960 symbols,
 *                        and a symbol is 16 microseconds. The scan will occur
 *                        for ((2^duration) + 1) scan periods.  The value of
 *                        duration must be less than 15. The time corresponding
 *                        to the first few values are as follows: 0 = 31 msec,
 *                        1 = 46 msec, 2 = 77 msec, 3 = 138 msec, 4 = 261 msec,
 *                        5 = 507 msec, 6 = 998 msec. This parameter is set by
 *                        default to 0.
 *
 * @return   An ::EmberStatus value of EMBER_SUCCESS if the frequency agility
 * parameters were correctly set. An ::EmberStatus value of EMBER_INVALID_CALL
 * if the node has not been not started yet, or if it has been started as
 * controller, or the current network is not an RF4CE network, or the node is
 * currently busy with some discovery or pairing process, or some of the passed
 * parameters are not valid.
 */
EmberStatus emberRf4ceSetFrequencyAgilityParameters(int8u rssiWindowSize,
                                                    int8u channelChangeReads,
                                                    int8s rssiThreshold,
                                                    int16u readInterval,
                                                    int8u readDuration);

/** @brief  Set the discovery LQI threshold parameter.
 *
 * @param threshold       The LQI threshold below which discovery requests will
 *                        be rejected.
 *
 * @return   An ::EmberStatus value of EMBER_SUCCESS if the discovery LQI
 * threshold was successfully set. Another ::EmberStatus value indicating the
 * specific error occurred otherwise.
 */
EmberStatus emberRf4ceSetDiscoveryLqiThreshold(int8u threshold);

#endif //__RF4CE_API_H__
