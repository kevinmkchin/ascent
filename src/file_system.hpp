#pragma once

#include "common.hpp"
#include <string>

/**
    Types to hold data in memory
*/

/** Handle for a file in memory */
struct binary_file_handle_t
{
    u64  size    = 0;        // size of file in memory
    void*   memory  = nullptr;  // pointer to file in memory
};

/** Handle for an UNSIGNED BYTE bitmap in memory */
struct bitmap_handle_t : binary_file_handle_t
{
    u32  width = 0;      // image width
    u32  height = 0;     // image height
    u8   bit_depth = 0;  // bit depth of bitmap in bytes (e.g. bit depth = 3 means there are 3 bytes in the bitmap per pixel)
};


/**
    File system functions 
*/

void free_file_binary(binary_file_handle_t& binary_file_to_free);
void read_file_binary(binary_file_handle_t& mem_to_read_to, const char* file_path);
std::string read_file_string(const char* file_path);
void free_image(bitmap_handle_t& image_handle);
void read_image(bitmap_handle_t& image_handle, const char* image_file_path);