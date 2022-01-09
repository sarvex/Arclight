/*
 *	 Copyright (c) 2021 - Arclight Team
 *
 *	 This file is part of Arclight. All rights reserved.
 *
 *	 filestreamimpl.cpp
 */

#include "filestreamimpl.hpp"
#include "filesystem/file.hpp"
#include "util/assert.hpp"
#include "types.hpp"


FileStreamImpl::FileStreamImpl(File& file) : file(file) {}


SizeT FileStreamImpl::read(void* dest, SizeT size) {

	arc_assert(dest != nullptr, "Destination is null");
	return file.read(std::span<u8>{static_cast<u8*>(dest), size});

}

SizeT FileStreamImpl::write(const void* src, SizeT size) {

	arc_assert(src != nullptr, "Source is null");

	file.write(std::span<const u8>{static_cast<const u8*>(src), size});
	return size;

}



SizeT FileStreamImpl::seek(i64 offset, StreamBase::SeekMode mode) {

	SizeT pos = 0;

	switch (mode) {

	case StreamBase::SeekMode::Begin:
		arc_assert(offset >= 0, "Cannot seek before the start of the stream");
		pos = offset;
		break;

	case StreamBase::SeekMode::Current:
		pos = getPosition() + offset;
		break;

	case StreamBase::SeekMode::End:
		arc_assert(offset >= 0, "Cannot seek after the end of the stream");
		pos = getSize() - offset;
		break;

	}

	file.seek(pos);

	return pos;

}

SizeT FileStreamImpl::getPosition() const {
	return file.tell();
}



SizeT FileStreamImpl::getSize() const {
	return file.getFileSize();
}


bool FileStreamImpl::isOpen() const {
	return file.isOpen();
}


bool FileStreamImpl::reachedEnd() const {

	auto pos = getPosition();
	return pos == file.getFileSize() || pos == -1;

}