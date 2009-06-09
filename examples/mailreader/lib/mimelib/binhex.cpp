// binhex.cpp

#define DW_IMPLEMENTATION

#include <string.h>
#include <mimelib/binhex.h>

const char * const kPreamble =
    "(This file must be converted with BinHex 4.0)";

const char kBinhexChars[] =
  "!\"#$%&'()*+,-012345689@ABCDEFGHIJKLMNPQRSTUVXYZ[`abcdefhijklmpqr";
//            1         2         3         4         5         6
// 0 123456789012345678901234567890123456789012345678901234567890123

#define DONE 0x7F
#define SKIP 0x7E
#define FAIL 0x7D

const char kBinhexTable[] = {
// 0x00
    SKIP, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
    FAIL, SKIP, SKIP, FAIL, FAIL, SKIP, FAIL, FAIL,
// 0x10
    FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
    FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
// 0x20
    SKIP, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
    0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, FAIL, FAIL,
// 0x30
    0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, FAIL,
    0x14, 0x15, DONE, FAIL, FAIL, FAIL, FAIL, FAIL,
// 0x40
    0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D,
    0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, FAIL,
// 0x50
    0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, FAIL,
    0x2C, 0x2D, 0x2E, 0x2F, FAIL, FAIL, FAIL, FAIL,
// 0x60
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, FAIL,
    0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, FAIL, FAIL,
// 0x70
    0x3D, 0x3E, 0x3F, FAIL, FAIL, FAIL, FAIL, FAIL,
    FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
// 0x80
    FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
    FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
// 0x90
    FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
    FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
// 0xA0
    FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
    FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
// 0xB0
    FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
    FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
// 0xC0
    FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
    FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
// 0xD0
    FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
    FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
// 0xE0
    FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
    FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
// 0xF0
    FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
    FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
};

static DwUint16 kCrcTable[] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
    0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
    0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
    0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
    0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
    0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
    0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
    0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
    0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
    0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
    0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
    0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
    0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
    0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
    0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
    0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
    0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
    0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
    0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
    0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
    0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
    0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
    0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
};


inline DwUint16 UPDATE_CRC(DwUint16 crc, int ch)
{
    int idx = ((crc >> 8) ^ ch) & 0xff;
    return (DwUint16) ((crc << 8) ^ kCrcTable[idx]);
}


struct DwBinhexEncodeCtx {
    DwBinhexEncodeCtx();
    void PutChar(int aChar);
    void EncodeChar(int aChar);
    void Finalize();
    DwString mBuffer;
    int      mRunLen;
    int      mLastChar;
    char     mScratch[8];      // for 8-bit to ASCII conversion
    int      mScratchPos;      // number of chars in mScratch
    int      mLineLength;
};


DwBinhexEncodeCtx::DwBinhexEncodeCtx()
{
    mRunLen     =  1;
    mLastChar   = -1;
    mScratchPos =  0;
    mLineLength =  0;
}


inline void DwBinhexEncodeCtx::PutChar(int aChar)
{
    if (mLineLength == 64) {
        mBuffer.append(DW_EOL);
        mLineLength = 0;
    }
    mBuffer.append((size_t) 1, (char) aChar);
    ++mLineLength;
}


void DwBinhexEncodeCtx::EncodeChar(int aChar)
{
    // If we're in a run...
    if (aChar == mLastChar && mRunLen < 255) {
        ++mRunLen;
        return;
    }
    // If we are not in a run, and have not just finished a run...
    if (mRunLen == 1) {
        // Output the current character, but watch for 0x90, which must be
        // output as the two character sequence 0x90 0x00
        if (aChar != 0x90) {
            mScratch[mScratchPos++] = (DwUint8) aChar;
        }
        else {
            mScratch[mScratchPos++] = (DwUint8) 0x90;
            mScratch[mScratchPos++] = (DwUint8) 0x00;
        }
    }
    // If we just finished a run of length 2 ...
    else if (mRunLen == 2) {
        // Output the last character, but watch for 0x90, which must be
        // output as the two character sequence 0x90 0x00
        if (mLastChar != 0x90) {
            mScratch[mScratchPos++] = (DwUint8) mLastChar;
        }
        else {
            mScratch[mScratchPos++] = (DwUint8) 0x90;
            mScratch[mScratchPos++] = (DwUint8) 0x00;
        }
        // Output the current character, but watch for 0x90, which must be
        // output as the two character sequence 0x90 0x00
        if (aChar != 0x90) {
            mScratch[mScratchPos++] = (DwUint8) aChar;
        }
        else {
            mScratch[mScratchPos++] = (DwUint8) 0x90;
            mScratch[mScratchPos++] = (DwUint8) 0x00;
        }
    }
    // If we just finished a run of length greater than 2 ...
    else /* if (mRunLen > 2) */ {
        // Output the run length code
        mScratch[mScratchPos++] = (DwUint8) 0x90;
        mScratch[mScratchPos++] = (DwUint8) mRunLen;
        // Output the current character, but watch for 0x90, which must be
        // output as the two character sequence 0x90 0x00
        if (aChar != 0x90) {
            mScratch[mScratchPos++] = (DwUint8) aChar;
        }
        else {
            mScratch[mScratchPos++] = (DwUint8) 0x90;
            mScratch[mScratchPos++] = (DwUint8) 0x00;
        }
    }
    mRunLen = 1;
    mLastChar = aChar;
    while (mScratchPos >= 3) {
        int n = mScratch[0] >> 2;
        PutChar(kBinhexChars[n&0x3f]);
        n = (mScratch[0] << 4) | ((mScratch[1] >> 4) & 0x0f);
        PutChar(kBinhexChars[n&0x3f]);
        n = (mScratch[1] << 2) | ((mScratch[2] >> 6) & 0x03);
        PutChar(kBinhexChars[n&0x3f]);
        n = mScratch[2];
        PutChar(kBinhexChars[n&0x3f]);
        for (int i=0; i < mScratchPos-3; ++i) {
            mScratch[i] = mScratch[i+3];
        }
        mScratchPos -= 3;
    }
}


void DwBinhexEncodeCtx::Finalize()
{
    if (mRunLen == 1) {
    }
    else if (mRunLen == 2) {
        // Output the last character, but watch for 0x90, which must be
        // output as the two character sequence 0x90 0x00
        if (mLastChar != 0x90) {
            mScratch[mScratchPos++] = (DwUint8) mLastChar;
        }
        else {
            mScratch[mScratchPos++] = (DwUint8) 0x90;
            mScratch[mScratchPos++] = (DwUint8) 0x00;
        }
    }
    else /* if aCtx->mRunLen > 2) */ {
        // Output the run length code
        mScratch[mScratchPos++] = (DwUint8) 0x90;
        mScratch[mScratchPos++] = (DwUint8) mRunLen;
    }
    int n;
    while (mScratchPos >= 3) {
        n = mScratch[0] >> 2;
        PutChar(kBinhexChars[n&0x3f]);
        n = (mScratch[0] << 4) | ((mScratch[1] >> 4) & 0x0f);
        PutChar(kBinhexChars[n&0x3f]);
        n = (mScratch[1] << 2) | ((mScratch[2] >> 6) & 0x03);
        PutChar(kBinhexChars[n&0x3f]);
        n = mScratch[2];
        PutChar(kBinhexChars[n&0x3f]);
        for (int i=0; i < mScratchPos-3; ++i) {
            mScratch[i] = mScratch[i+3];
        }
        mScratchPos -= 3;
    }
    switch (mScratchPos) {
    case 0:
        break;
    case 1:
        n = mScratch[0] >> 2;
        PutChar(kBinhexChars[n&0x3f]);
        n = (mScratch[0] << 4);
        PutChar(kBinhexChars[n&0x3f]);        
    case 2:
        n = mScratch[0] >> 2;
        PutChar(kBinhexChars[n&0x3f]);
        n = (mScratch[0] << 4) | ((mScratch[1] >> 4) & 0x0f);
        PutChar(kBinhexChars[n&0x3f]);
        n = (mScratch[1] << 2);
        PutChar(kBinhexChars[n&0x3f]);
    }
}

#if 0

//============================================================================


struct DwBinhexDecodeCtx {
    DwBinhexDecodeCtx();
    int GetChar();
    int DecodeChar();
    DwString mBinhexChars;
    size_t   mPos;
    int      mRunLen;
    int      mLastChar;
    DwUint8  mScratch[4];
    int      mScratchPos;
    int      mScratchCount;
    int      mError;
};


DwBinhexDecodeCtx::DwBinhexDecodeCtx()
{
    mPos = 0;
    mRunLen = 1;
    mLastChar = -1;
    mScratch[0] = 0;
    mScratch[1] = 0;
    mScratch[2] = 0;
    mScratch[3] = 0;
    mScratchPos = 0;
    mScratchCount = 0;
    mError = 0;
}


int DwBinhexDecodeCtx::GetChar()
{
    // Refill the scratch buffer, if necessary
    if (mScratchCount == 0) {
        // Get up to four ASCII chars
        char cc[4];
        size_t k = 0;
        size_t len = mBinhexChars.length();
        const DwString& binhexChars = mBinhexChars;
        while (k < (size_t) 4 && mPos < len) {
            int ch = binhexChars[mPos++] & 0xff;
            ch = kBinhexTable[ch];
            switch (ch) {
            case DONE:
                goto BREAK_1;
            case SKIP:
                break;
            case FAIL:
                mError = 1; // error!
                return -1;
            default:
                cc[k++] = (char) ch;
                break;
            }
        }
        BREAK_1:
        // Convert the ASCII chars to 8-bit chars
        mScratch[0] = 0;
        mScratch[1] = 0;
        mScratch[2] = 0;
        mScratchCount = 0;
        mScratchPos = 0;
        switch (k) {
        case 4:
            mScratch[2] |= (DwUint8) (cc[3] & 0x3f);
            // fall through
        case 3:
            mScratch[2] |= (DwUint8) (cc[2] << 6);
            mScratch[1] |= (DwUint8) ((cc[2] >> 2) & 0x0f);
            ++mScratchCount;
            // fall through
        case 2:
            mScratch[1] |= (DwUint8) (cc[1] << 4);
            mScratch[0] |= (DwUint8) ((cc[1] >> 4) & 0x03);
            ++mScratchCount;
            // fall through
        case 1:
            mScratch[0] |= (DwUint8) (cc[0] << 2);
            ++mScratchCount;
        case 0:
            break;
        }
    }
    // Return an 8-bit char, or -1 if there are no more chars
    int ch;
    if (mScratchCount > 0) {
        --mScratchCount;
        ch = mScratch[mScratchPos++] & 0xff;
    }
    else {
        ch = -1;
    }
    return ch;
}


int DwBinhexDecodeCtx::DecodeChar()
{
    int ch;
    if (mRunLen > 1) {
        ch = mLastChar;
        --mRunLen;
    }
    else /* if (mRunLen == 1) */ {
        ch = GetChar();
        // 0x90 is the escape character
        if ((ch & 0xff) == 0x90) {
            ch = GetChar();
            if (ch == -1) {
                // end of buffer or illegal character
                mError = 1; // error!
            }
            else if (ch == 0) {
                // 0x90 0x00 is decoded to 0x90
                ch = 0x90;
                mRunLen = 1;
            }
            else if (ch == 1) {
                // Could be interpreted as a run of length 1, but in all
                // likelihood, it's an error.
                mError = 1; // error!
                ch = -1;
                mRunLen = 1;
            }
            else if (ch >= 2) {
                // 0x90 n indicates a run of length n
                mRunLen = ch - 1;
                ch = mLastChar;
            }
        }
    }
    mLastChar = ch;
    return ch;
}


//============================================================================


DwBinhex::DwBinhex()
{
    Initialize();
}
#endif // 0

DwBinhex::~DwBinhex()
{
}

#if 0
void DwBinhex::Initialize()
{
    memset(mFileName, 0, sizeof(mFileName));
    memset(mFileType, 0, sizeof(mFileType));
    memset(mFileCreator, 0, sizeof(mFileCreator));
    mFlag1 = 0;
    mFlag2 = 0;
    mDataFork = mResourceFork = mBinhexChars = "";
}


void DwBinhex::SetFileName(const char* aName)
{
    strncpy(mFileName, aName, 64);
    mFileName[63] = 0;
}


const char* DwBinhex::FileName() const
{
    return mFileName;
}


void DwBinhex::SetFileType(const char* aType)
{
    memcpy(mFileType, aType, 4);
}


void DwBinhex::FileType(char* aBuf) const
{
    memcpy(aBuf, mFileType, 4);
}


void DwBinhex::SetFileCreator(const char* aCreator)
{
    memcpy(mFileCreator, aCreator, 4);
}

void DwBinhex::FileCreator(char* aBuf) const
{
    memcpy(aBuf, mFileCreator, 4);
}

 
void DwBinhex::SetFlag1(DwUint8 aFlag)
{
    mFlag1 = aFlag;
}


DwUint8 DwBinhex::Flag1() const
{
    return mFlag1;
}


void DwBinhex::SetFlag2(DwUint8 aFlag)
{
    mFlag2 = aFlag;
}


DwUint8 DwBinhex::Flag2() const
{
    return mFlag2;
}


void DwBinhex::SetDataFork(const DwString& aStr)
{
    mDataFork = aStr;
}


const DwString& DwBinhex::DataFork() const
{
    return mDataFork;
}


void DwBinhex::SetResourceFork(const DwString& aStr)
{
    mResourceFork = aStr;
}


const DwString& DwBinhex::ResourceFork() const
{
    return mResourceFork;
}


void DwBinhex::SetBinhexChars(const DwString& aStr)
{
    mBinhexChars = aStr;
}


const DwString& DwBinhex::BinhexChars() const
{
    return mBinhexChars;
}


void DwBinhex::Encode()
{
    size_t bufSize = (mResourceFork.length()+2)/3*4
        + (mDataFork.length()+2)/3*4 + 27 + strlen(mFileName);
    DwBinhexEncodeCtx ctx;
    ctx.mBuffer.reserve(bufSize);
    ctx.mBuffer.assign(kPreamble);
    ctx.mBuffer.append(DW_EOL);
    ctx.mBuffer.append(1, ':');
    ++ctx.mLineLength;
    DwUint16 crc = 0;
    size_t fileNameLen = strlen(mFileName);
    int ch = fileNameLen;
    crc = UPDATE_CRC(crc, ch);
    ctx.EncodeChar(ch);
    // File name
    size_t j;
    for (j=0; j < fileNameLen; ++j) {
        ch = mFileName[j] & 0xff;
        crc = UPDATE_CRC(crc, ch);
        ctx.EncodeChar(ch);
    }
    // Version
    ch = 0;
    crc = UPDATE_CRC(crc, ch);
    ctx.EncodeChar(ch);
    // File type
    for (j=0; j < (size_t) 4; ++j) {
        ch = mFileType[j] & 0xff;
        crc = UPDATE_CRC(crc, ch);
        ctx.EncodeChar(ch);
    }
    // File creator
    for (j=0; j < (size_t) 4; ++j) {
        ch = mFileCreator[j] & 0xff;
        crc = UPDATE_CRC(crc, ch);
        ctx.EncodeChar(ch);
    }
    // Flags
    ch = mFlag1 & 0xff;
    crc = UPDATE_CRC(crc, ch);
    ctx.EncodeChar(ch);
    ch = mFlag2 & 0xff;
    crc = UPDATE_CRC(crc, ch);
    ctx.EncodeChar(ch);
    // Data fork length
    DwUint32 len = (DwUint32) mDataFork.length();
    ch = (len >> 24) & 0xff;
    crc = UPDATE_CRC(crc, ch);
    ctx.EncodeChar(ch);
    ch = (len >> 16) & 0xff;
    crc = UPDATE_CRC(crc, ch);
    ctx.EncodeChar(ch);
    ch = (len >>  8) & 0xff;
    crc = UPDATE_CRC(crc, ch);
    ctx.EncodeChar(ch);
    ch = len & 0xff;
    crc = UPDATE_CRC(crc, ch);
    ctx.EncodeChar(ch);
    // Resource fork length
    len = mResourceFork.length();
    ch = (len >> 24) & 0xff;
    crc = UPDATE_CRC(crc, ch);
    ctx.EncodeChar(ch);
    ch = (len >> 16) & 0xff;
    crc = UPDATE_CRC(crc, ch);
    ctx.EncodeChar(ch);
    ch = (len >>  8) & 0xff;
    crc = UPDATE_CRC(crc, ch);
    ctx.EncodeChar(ch);
    ch = len & 0xff;
    crc = UPDATE_CRC(crc, ch);
    ctx.EncodeChar(ch);
    // Header CRC
    ch = (crc >> 8) & 0xff;
    ctx.EncodeChar(ch);
    ch = crc & 0xff;
    ctx.EncodeChar(ch);
    //===== End of header =====

    // Data fork
    crc = 0;
    size_t dataForkLen = mDataFork.length();
    for (j=0; j < dataForkLen; ++j) {
        ch = mDataFork[j] & 0xff;
        crc = UPDATE_CRC(crc, ch);
        ctx.EncodeChar(ch);
    }
    // Data fork CRC
    ch = (crc >> 8) & 0xff;
    ctx.EncodeChar(ch);
    ch = crc & 0xff;
    ctx.EncodeChar(ch);

    // Resource fork
    crc = 0;
    size_t rsrcForkLen = mResourceFork.length();
    for (j=0; j < rsrcForkLen; ++j) {
        ch = mResourceFork[j] & 0xff;
        crc = UPDATE_CRC(crc, ch);
        ctx.EncodeChar(ch);
    }
    // Resource fork CRC
    ch = (crc >> 8) & 0xff;
    ctx.EncodeChar(ch);
    ch = crc & 0xff;
    ctx.EncodeChar(ch);

    ctx.Finalize();

    ctx.mBuffer.append(1, ':');
    ctx.mBuffer.append(DW_EOL);

    mBinhexChars = ctx.mBuffer;
}


int DwBinhex::Decode()
{
    // Initialize
    memset(mFileName, 0, sizeof(mFileName));
    memset(mFileType, 0, sizeof(mFileType));
    memset(mFileCreator, 0, sizeof(mFileCreator));
    mFlag1 = 0;
    mFlag2 = 0;
    mDataFork = mResourceFork = "";

    DwBinhexDecodeCtx ctx;
    ctx.mBinhexChars = mBinhexChars;
	// Find the preamble
    ctx.mPos = ctx.mBinhexChars.find("(This file must be converted "
        "with BinHex", 0);
    if (ctx.mPos == DwString::npos) {
        return -1; // error!
    }
    ctx.mPos += 40;
    // Advance to just past the colon
    ctx.mPos = ctx.mBinhexChars.find((char) ':', ctx.mPos);
    if (ctx.mPos == DwString::npos) {
        return -1; // error!
    }
    ++ctx.mPos;
    DwUint16 crc = 0;
    // File name length
    int ch = ctx.DecodeChar();
    if (ch < 1 || 63 < ch) {
        return -1; // error!
    }
    crc = UPDATE_CRC(crc, ch);
    size_t fileNameLen = (size_t) ch;
    // File name
    size_t j;
    for (j=0; j < fileNameLen; ++j) {
        ch = ctx.DecodeChar();
        if (ch == -1) {
            return -1; // error!
        }
        crc = UPDATE_CRC(crc, ch);
        mFileName[j] = (char) ch;
    }
    // Version
    ch = ctx.DecodeChar();
    if (ch != 0) {
        return -1; // error!
    }
    crc = UPDATE_CRC(crc, ch);
    // File type
    for (j=0; j < (size_t) 4; ++j) {
        ch = ctx.DecodeChar();
        if (ch == -1) {
            return -1; // error!
        }
        crc = UPDATE_CRC(crc, ch);
        mFileType[j] = (char) ch;
    }
    // File creator
    for (j=0; j < (size_t) 4; ++j) {
        ch = ctx.DecodeChar();
        if (ch == -1) {
            return -1; // error!
        }
        crc = UPDATE_CRC(crc, ch);
        mFileCreator[j] = (char) ch;
    }
    // Flags
    ch = ctx.DecodeChar();
    if (ch == -1) {
        return -1; // error!
    }
    crc = UPDATE_CRC(crc, ch);
    mFlag1 = (DwUint8) ch;
    ch = ctx.DecodeChar();
    if (ch == -1) {
        return -1; // error!
    }
    crc = UPDATE_CRC(crc, ch);
    mFlag2 = (DwUint8) ch;
    // Data fork length
    ch = ctx.DecodeChar();
    if (ch == -1) {
        return -1; // error!
    }
    crc = UPDATE_CRC(crc, ch);
    DwUint32 dataForkLen = ch & 0xff;
    ch = ctx.DecodeChar();
    if (ch == -1) {
        return -1; // error!
    }
    crc = UPDATE_CRC(crc, ch);
    dataForkLen <<= 8;
    dataForkLen |= ch & 0xff;
    ch = ctx.DecodeChar();
    if (ch == -1) {
        return -1; // error!
    }
    crc = UPDATE_CRC(crc, ch);
    dataForkLen <<= 8;
    dataForkLen |= ch & 0xff;
    ch = ctx.DecodeChar();
    if (ch == -1) {
        return -1; // error!
    }
    crc = UPDATE_CRC(crc, ch);
    dataForkLen <<= 8;
    dataForkLen |= ch & 0xff;
    // Resource fork length
    ch = ctx.DecodeChar();
    if (ch == -1) {
        return -1; // error!
    }
    crc = UPDATE_CRC(crc, ch);
    DwUint32 rsrcForkLen = ch & 0xff;
    ch = ctx.DecodeChar();
    if (ch == -1) {
        return -1; // error!
    }
    crc = UPDATE_CRC(crc, ch);
    rsrcForkLen <<= 8;
    rsrcForkLen |= ch & 0xff;
    ch = ctx.DecodeChar();
    if (ch == -1) {
        return -1; // error!
    }
    crc = UPDATE_CRC(crc, ch);
    rsrcForkLen <<= 8;
    rsrcForkLen |= ch & 0xff;
    ch = ctx.DecodeChar();
    if (ch == -1) {
        return -1; // error!
    }
    crc = UPDATE_CRC(crc, ch);
    rsrcForkLen <<= 8;
    rsrcForkLen |= ch & 0xff;
    // Header CRC
    ch = ctx.DecodeChar();
    if (ch == -1) {
        return -1; // error!
    }
    DwUint16 crc1 = (DwUint16) (ch & 0xff);
    ch = ctx.DecodeChar();
    if (ch == -1) {
        return -1; // error!
    }
    crc1 <<= 8;
    crc1 |= (DwUint16) (ch & 0xff);
    if (crc1 != crc) {
        return -1; // error!
    }

    // Data fork
    crc = 0;
    for (j=0; j < dataForkLen; ++j) {
        ch = ctx.DecodeChar();
        if (ch == -1) {
            return -1; // error!
        }
        crc = UPDATE_CRC(crc, ch);
        mDataFork.append((size_t) 1, (char) ch);
    }
    ch = ctx.DecodeChar();
    if (ch == -1) {
        return -1; // error!
    }
    crc1 = (DwUint16) (ch & 0xff);
    ch = ctx.DecodeChar();
    if (ch == -1) {
        return -1; // error!
    }
    crc1 <<= 8;
    crc1 |= (DwUint16) (ch & 0xff);
    if (crc1 != crc) {
        mDataFork = "";
        return -1; // error!
    }

    // Resource fork
    crc = 0;
    for (j=0; j < rsrcForkLen; ++j) {
        ch = ctx.DecodeChar();
        if (ch == -1) {
            return -1; // error!
        }
        crc = UPDATE_CRC(crc, ch);
        mResourceFork.append((size_t) 1, (char) ch);
    }
    ch = ctx.DecodeChar();
    if (ch == -1) {
        return -1; // error!
    }
    crc1 = (DwUint16) (ch & 0xff);
    ch = ctx.DecodeChar();
    if (ch == -1) {
        return -1; // error!
    }
    crc1 <<= 8;
    crc1 |= (DwUint16) (ch & 0xff);
    if (crc1 != crc) {
        mResourceFork = "";
        return -1; // error!
    }
    return 0;
}

#endif
