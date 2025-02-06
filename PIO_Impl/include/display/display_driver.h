#pragma once
#include <cstdint>

class IDisplay {
public:
    virtual ~IDisplay() = default;
    
    // Initialization
    virtual bool init() = 0;
    
    // Basic drawing operations
    virtual void clear() = 0;
    virtual void setBackgroundColor(uint16_t color) = 0;
    
    // Screen properties
    virtual uint16_t getWidth() = 0;
    virtual uint16_t getHeight() = 0;
    
    // Common color definitions
    static constexpr uint16_t BLACK = 0x0000;
    static constexpr uint16_t WHITE = 0xFFFF;
    static constexpr uint16_t RED = 0xF800;
    static constexpr uint16_t GREEN = 0x07E0;
    static constexpr uint16_t BLUE = 0x001F;
};
