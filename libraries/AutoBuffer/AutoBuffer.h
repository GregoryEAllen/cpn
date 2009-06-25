/** \file
 * Header for the AutoBuffer class. The concept of the
 * auto buffer is very simplar to the auto_ptr. Basically
 * a class who's job it is to maintain the lifetime
 * of a buffer.
 */

#ifndef AUTOBUFFER_H
#define AUTOBUFFER_H

/**
 * An AutoBuffer handles a dynamically sized buffer of raw memory
 * automatically. This class also provides a few convenience
 * methods for dealing with the raw memory.
 */
class AutoBuffer {
public:
	typedef unsigned long ulong;
	AutoBuffer(ulong initialsize);
	AutoBuffer(const AutoBuffer& other);
	AutoBuffer(void* other, ulong othersize);
	~AutoBuffer();
	
	AutoBuffer& operator=(const AutoBuffer& other);
	/**
	 * \return a pointer to the buffer.
	 */
	void* GetBuffer() { return buffer; }

	void* GetBuffer(ulong offset);

	operator void*() { return buffer; }

	/**
	 * \return the current size of the buffer.
	 */
	ulong GetSize() const { return size; }

	/**
	 * Change the size of this buffer. If newsize is larger than
	 * current size then the buffer will be extended. The data
	 * already in buffer will remain unchanged. See realloc.
	 * If newsize is smaller then the buffer will be truncated.
	 * \param newsize the new size in bytes for this buffer
	 */
	void ChangeSize(ulong newsize);

	/**
	 * Copy othersize bytes from other into this buffer starting at
	 * offset. If there is not enough space this buffer will try to expand.
	 * \param other the other memory buffer
	 * \param othersize the number of bytes to copy
	 * \param offset the offset into us to start copying at
	 */
	void Put(const void* other, const ulong othersize, const ulong offset=0);

	/**
	 * Copy up to othersize bytes into other starting at offset.
	 * If othersize+offset is larger than our size then only copy up to the
	 * end of us. Returns the number of bytes actually copied.
	 * \param other the other buffer to copy into
	 * \param othersize the number of bytes to copy into other
	 * \param offset where to start copying in us
	 * \return the number of bytes actually copied
	 */
	ulong Get(void* other, const ulong othersize, const ulong offset=0) const;

private:
	void* buffer;
	ulong size;
};

#endif
