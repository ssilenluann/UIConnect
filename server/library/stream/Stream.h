#ifndef STREAM_STREAM_H
#define STREAM_STREAM_H

#include <memory>
#include "../ByteArray.h"

class Stream
{
public:
    typedef std::shared_ptr<Stream> ptr;

    virtual ~Stream() {}

    /**
     * @param[in] buffer recv buffer
     * @param[in] length size
     * 
     * @return 
     *      @retval > 0 actual recv size
     *      @retval = 0 closed
     *      @retval < 0 error
    */
    virtual int read(void* buffer, size_t length) = 0;

    /**
     * @param[in] ba recv buffer
     * @param[in] length size
     * 
     * @return 
     *      @retval > 0 actual recv size
     *      @retval = 0 closed
     *      @retval < 0 error
    */
    virtual int read(ByteArray::ptr ba, size_t length) = 0;

    /**
     * @brief read data with specified length
     * @param[in] buffer recv buffer
     * @param[in] length expected read size
     * 
     * @return 
     *      @retval > 0 actual recv size
     *      @retval = 0 closed
     *      @retval < 0 error
    */
    virtual int readFixedSize(void* buffer, size_t length);

    /**
     * @brief read data with specified length
     * @param[in] ba recv buffer
     * @param[in] length expected read size
     * 
     * @return 
     *      @retval > 0 actual recv size
     *      @retval = 0 closed
     *      @retval < 0 error
    */
    virtual int readFixedSize(ByteArray::ptr ba, size_t length);


    /**
     * @param[in] buffer wirte buffer
     * @param[in] length size
     * 
     * @return 
     *      @retval > 0 actual wirte size
     *      @retval = 0 closed
     *      @retval < 0 error
    */
    virtual int wirte(void* buffer, size_t length) = 0;

    /**
     * @param[in] ba wirte buffer
     * @param[in] length size
     * 
     * @return 
     *      @retval > 0 actual wirte size
     *      @retval = 0 closed
     *      @retval < 0 error
    */
    virtual int wirte(ByteArray::ptr ba, size_t length) = 0;

    /**
     * @brief wirte data with specified length
     * @param[in] buffer wirte buffer
     * @param[in] length expected wirte size
     * 
     * @return 
     *      @retval > 0 actual wirte size
     *      @retval = 0 closed
     *      @retval < 0 error
    */
    virtual int wirteFixedSize(void* buffer, size_t length);

    /**
     * @brief wirte data with specified length
     * @param[in] ba wirte buffer
     * @param[in] length expected wirte size
     * 
     * @return 
     *      @retval > 0 actual wirte size
     *      @retval = 0 closed
     *      @retval < 0 error
    */
    virtual int wirteFixedSize(ByteArray::ptr ba, size_t length);

    virtual void close() = 0;
};
#endif