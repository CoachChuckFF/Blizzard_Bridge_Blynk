//////////////////////////////////////////////////////////////
//
//	Copyright Artistic Licence Holdings Ltd 2012-2015
//	Author:	Wayne Howell
//	Email:	Support@ArtisticLicence.com
//
//	This file contains all key defines and structures for sACN E1.31
//
//////////////////////////////////////////////////////////////

#ifndef _SACN_H_
#define _SACN_H_

#define uchar unsigned char
#define ushort unsigned short int
#define ulong unsigned int
// 8, 16, 32 bit fields

#define RootLayerPreambleSizeSize		2
#define RootLayerPostambleSizeSize		2
#define RootLayerAcnPacketIdentifierSize	12
#define RootLayerFlagsAndLengthSize		2
#define RootLayerVectorSize			4
#define RootLayerCidSize			16


typedef struct {
	uchar	PreambleSize[RootLayerPreambleSizeSize];
	uchar	PostambleSize[RootLayerPostambleSizeSize];
	uchar	AcnPacketIdentifier[RootLayerAcnPacketIdentifierSize];
	uchar	FlagsAndLength[RootLayerFlagsAndLengthSize];
	uchar	Vector[RootLayerVectorSize];
	uchar	Cid[RootLayerCidSize];
} TsAcnRootLayer;


#define FramingLayerFlagsAndLengthSize	2
#define FramingLayerVectorSize		4
#define FramingLayerSourceNameSize	64
#define FramingLayerPrioritySize	1
#define FramingLayerReservedSize	2
#define FramingLayerSequenceNumberSize	1
#define FramingLayerOptionsSize		1
#define FramingLayerUniverseSize	2


typedef struct {
	uchar	FlagsAndLength[FramingLayerFlagsAndLengthSize];
	uchar	Vector[FramingLayerVectorSize];
	uchar	SourceName[FramingLayerSourceNameSize];
	uchar	Priority[FramingLayerPrioritySize];
	uchar	Reserved[FramingLayerReservedSize];
	uchar	SequenceNumber[FramingLayerSequenceNumberSize];
	uchar	Options[FramingLayerOptionsSize];
	uchar	Universe[FramingLayerUniverseSize];
} TsAcnFramingLayer;


#define DmpLayerFlagsAndLengthSize		2
#define DmpLayerVectorSize			1
#define DmpLayerAddressAndDataTypeSize		1
#define DmpLayerFirstPropertyAddressSize	2
#define DmpLayerAddressIncrementSize		2
#define DmpLayerPropertyValueCountSize		2
#define DmpLayerStartCodeSize 			1
#define DmpLayerPropertyValuesSize  		512

typedef struct {
	uchar	FlagsAndLength[DmpLayerFlagsAndLengthSize];
	uchar	Vector[DmpLayerVectorSize];
	uchar	AddressAndDataType[DmpLayerAddressAndDataTypeSize];
	uchar	FirstPropertyAddress[DmpLayerFirstPropertyAddressSize];
	uchar	AddressIncrement[DmpLayerAddressIncrementSize];
	uchar	PropertyValueCount[DmpLayerPropertyValueCountSize];
	uchar	StartCode[DmpLayerStartCodeSize];
	uchar	PropertyValues[DmpLayerPropertyValuesSize];
} TsAcnDmpLayer;


typedef struct {
	TsAcnRootLayer		RootLayer;
	TsAcnFramingLayer	FramingLayer;
	TsAcnDmpLayer		DmpLayer;

} TsAcn;



#endif

