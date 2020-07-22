! message file for the Mtools suite of programs
! This file is compiled directly on VMS systems, using the message
! facility. On other systems, other files are generated automatically
! from this file. THIS IS THE DEFINITIVE FILE FOR ERROR MESSAGES.
! The format should bre reasonably evident. Each message is accessed
! from the program using a symbol MTL__<msg-id>, eg MTL__ESTABLISHED
!

.TITLE    Mtools error and informational messages
.IDENT    'Version 1.00'

! the number should be a unique 11-bit number on VMS system (bit 12 must be 1)
! the prefix is what is used to make symbol names visible in the program

.FACILITY MTOOLS, 1 /prefix=MTL__


! choose warning for the SCI messages. No particular reason, but
! it makes the bottom 3 bits 0.
! each message is valid for 8 codes, but the message utility creates
! the variables (global values) with a particular severity.
! we'll override the severity when we deliver the message

.SEVERITY WARNING

! tie the SCI errors to particular numbers (remember bottom three
! bits are severity)

.base 0
SCI000 <Successful SCI transaction>
SCI001 <Unknown Message type>
SCI002 <Message Length incorrect>
SCI003 <Generic SCI error>
SCI004 <Host disconnected>
SCI005 <Bad Password>
SCI006 <Message no longer in debug queue>
SCI007 <>
SCI008 <request too long>
SCI009 <reply too long>
SCI010 <No such tag>
SCI011 <No such peerway>
SCI012 <No such node>
SCI013 <No such volume>
SCI014 <No such node/drive>
SCI015 <No such file>
SCI016 <No such controller card>
SCI017 <No such block/link>
SCI018 <Node not a ControlFile>
SCI019 <>
SCI020 <Disk read/write error>
SCI021 <Global SCI write disabled>
SCI022 <SCI write disabled for this block>
SCI023 <Block mode incorrect>
SCI024 <Value outside limits>
SCI025 <Invalid configuration to write>
SCI026 <Too many Set Time messages>
.base 30
SCI030 <SCI table full>
SCI031 <SCI table address already in use>
SCI032 <SCI not synchronised with host>
SCI033 <A write has been attempted while polling disabled>
SCI034 <The SCI table contains another entry for this block variable...>
.base 40
SCI040 <Raw block transfer in progress>
SCI041 <Checksum bad on raw data write>
.base 51
SCI051 <The reply message may not have status information for all writes>
.base 60
SCI060 <Cannot change message format in Asynchronous Terminator mode>
.base 70
SCI070 <No transfers available and no transfers have timed out>
SCI071 <Transfer not active. Transfer timed out>
SCI072 <Unable to find a report that meets the request message criteria>
SCI073 <Report generation conflict>
SCI074 <File transfer message not allowed for this folder and file type>
SCI075 <Unknown error occurred>

.base 100
SCI100 <Error in field 0 of SCI request>
SCI101 <Error in field 1 of SCI request>
SCI102 <Error in field 2 of SCI request>
SCI103 <Error in field 3 of SCI request>
SCI104 <Error in field 4 of SCI request>
SCI105 <Error in field 5 of SCI request>
SCI106 <Error in field 6 of SCI request>
SCI107 <Error in field 7 of SCI request>
SCI108 <Error in field 8 of SCI request>
SCI109 <Error in field 9 of SCI request>
SCI110 <Error in field 10 of SCI request>
SCI111 <Error in field 11 of SCI request>
SCI112 <Error in field 12 of SCI request>
SCI113 <Error in field 13 of SCI request>
SCI114 <Error in field 14 of SCI request>
SCI115 <Error in field 15 of SCI request>

! and the Mtools messages - fix at base 128 so that we can build
! numbers into DCL scripts
! The success, warn, etc stuff in here is just for guidance - the
! actual severity handed to VMS is set by the program

.BASE 128

.SEVERITY SUCCESS
  SUCCESS   <Successful completion>

.SEVERITY INFORMATIONAL  ! for /verbose
  ESTABLISHED   <Established connection to the SCI>
  CLOSED        <Closed connection to SCI>
  TIMEOUT       <Timeout while awaiting SCI reply>
  BADCRC        <CRC error on message from SCI>
  BADFRAME      <Framing error on message from SCI>
  NOID          <Reply from SCI with no message id>
  WRONGID       <Reply from SCI with incorrect id>
  NOSTATUS      <Reply from SCI with no status field>
  CREATEID      <Created SCI ID file>

.SEVERITY WARN
  NOIDFILE      <No SCI message ID file>
  BADIDFILE     <No SCI ID in file>
  CANTCREATEID  <Cant create SCI ID file>
  TOOSHORT      <Packet too short to contain any data: ignored>
  BADCONTROL    <Unknown or unsupported controller type>

.SEVERITY FATAL
! config or options
  BADNUMBER       <Expected a number>
  OUTOFRANGE      <Qualifier value out of range>
  TOOLONG         <Qualifier string value too long>
  EXCLUSIVEOPT    <Qualifier repeated or mutually exclusive options specified>
  MISSINGOPT      <Not all required qualifiers specified>
  NOVALUE         <Qualifier does not take a value>
  NEEDVALUE       <Qualifier requires a value>
  PARAMTOOLONG    <Parameter string too long>
  BADBAUD         <Unsupported BAUD rate>
!used only by non-CLD version - DCL handles these
  CLGARBAGE       <Extraneous stuff on command-line ignored>
  BADOPT          <Unknown option specified - check documentation>
! comms errors
  NOCOMMS         <Cannot establish communications with the SCI>
  TOOMANYRETRIES  <Too many retries for SCI request>
  ! (I'm sure there's a VMS one we could use for this)
  OUTOFSTORE      <Out of memory>
  CANTOPENOUT     <Cannot open output file>
  CANTOPENIN      <Cannot open input file>
  NOCONFIG        <Cannot open config file>
  BADCONFIG       <Error in config file>
  BADRAWFILE      <Error in raw data file>
  NOTEVEN         <Hexstring of odd length>
! errors from the mtools applications
  BADOFFSET       <Offset indicates non-contiguous data>
  BADHEX          <Received invalid hex digit in reply>
  FILECHANGED     <File has changed during upload or download>
  GARBAGE         <Unparsable reply from SCI>
.END
