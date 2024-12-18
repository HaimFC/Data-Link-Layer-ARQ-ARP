#ifndef EthernetLab
#define EthernetLab
#define TXMODE 0
#define RXMODE 1

#if defined(PROGMEM)
    #define FLASH_PROGMEM PROGMEM
    #define FLASH_READ_DWORD(x) (pgm_read_dword_near(x))
#else
    #define FLASH_PROGMEM
    #define FLASH_READ_DWORD(x) (*(uint32_t*)(x))
#endif

static const uint32_t crc32_table[] FLASH_PROGMEM = {
    0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
    0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
    0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
    0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
};

int mode = TXMODE;

class CRC32
{
public:
    CRC32();
    void reset();
    void update(const uint8_t& data);
    
    template <typename Type>
    void update(const Type& data)
    {
        update(&data, 1);
    }
    
    template <typename Type>
    void update(const Type* data, size_t size)
    {
        size_t nBytes = size * sizeof(Type);
        const uint8_t* pData = (const uint8_t*)data;

        for (size_t i = 0; i < nBytes; i++)
        {
            update(pData[i]);
        }
    }
    
    uint32_t finalize() const;

    template <typename Type>
    static uint32_t calculate(const Type* data, size_t size)
    {
        CRC32 crc;
        crc.update(data, size);
        return crc.finalize();
    }

private:
    uint32_t _state = ~0L;

};

CRC32::CRC32()
{
    reset();
}


void CRC32::reset()
{
    _state = ~0L;
}


void CRC32::update(const uint8_t& data)
{
    // via http://forum.arduino.cc/index.php?topic=91179.0
    uint8_t tbl_idx = 0;

    tbl_idx = _state ^ (data >> (0 * 4));
    _state = FLASH_READ_DWORD(crc32_table + (tbl_idx & 0x0f)) ^ (_state >> 4);
    tbl_idx = _state ^ (data >> (1 * 4));
    _state = FLASH_READ_DWORD(crc32_table + (tbl_idx & 0x0f)) ^ (_state >> 4);
}


uint32_t CRC32::finalize() const
{
    return ~_state;
}

uint32_t calculateCRC(const char * payload, size_t payload_size){
    uint32_t checksum = CRC32::calculate(payload, payload_size);
    if (mode == TXMODE){
        return checksum; 
    }
    else{
        int random_number = random(0,100);
        if (random_number <= 10){
            checksum = checksum ^ 4;    
            }
        return checksum;
    }
}

setMode(int mod){
    mode = mod;
}

#endif /* EthernetLab */
