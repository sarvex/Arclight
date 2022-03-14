/*
 *	 Copyright (c) 2022 - Arclight Team
 *
 *	 This file is part of Arclight. All rights reserved.
 *
 *	 jpegdecoder.hpp
 */

#pragma once

#include "jpeg.hpp"
#include "decoder.hpp"
#include "image/image.hpp"
#include "stream/binaryreader.hpp"



class JPEGDecoder : public IImageDecoder {

public:

	JPEGDecoder() : validDecode(false), restartEnabled(false), decodingBuffer(reader) {}

	void decode(std::span<const u8> data);

	template<Pixel P>
	Image<P> getImage() const {

		if (!validDecode) {
			throw ImageDecoderException("Bad image decode");
		}

		return Image<P>();

	}

private:

	struct DecodingBuffer {

		DecodingBuffer(BinaryReader& reader) : data(0), size(0), empty(false), sink(reader) {}

		void reset();
		void saturate(u32 reqSize);
		u32 read(u32 count);
		void consume(u32 count);

		u32 data;
		i32 size;
		bool empty;
		BinaryReader& sink;

	};

	void parseApplicationSegment0();
	void parseHuffmanTable();
	void parseQuantizationTable();
	void parseRestartInterval();

	void parseFrameHeader();
	void parseScanHeader();

	SizeT calculateScanSize();
	void decodeScan();
	void decodeImage();
	void decodeBlock(JPEG::ImageComponent& component);

	u16 verifySegmentLength();

	JPEG::Scan scan;
	JPEG::Frame frame;
	JPEG::HuffmanTable dcHuffmanTables[4];
	JPEG::HuffmanTable acHuffmanTables[4];
	JPEG::QuantizationTable quantizationTables[4];

	bool restartEnabled;
	u32 restartInterval;

	BinaryReader reader;
	bool validDecode;

	DecodingBuffer decodingBuffer;

};