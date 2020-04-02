#pragma once

#include <fstream>
#include <vector>
#include <string>

using Byte=uint8_t;
using Short=uint16_t;
using Int=uint32_t;
using Long=uint64_t;

struct FILE_DATA{
    // number of decks in this file
    Long deckCount;    

    // deck number (001 to deckCount)
    std::vector<Int> field1;
    // deck number (duplicate?)
    std::vector<Int> field2;
    // series id (0=Yu-Gi-Oh!, 5=Vrains)
    std::vector<Int> field3;
    // signature card id
    std::vector<Int> field4;
    // character id
    std::vector<Int> field5;
    // dlc flag?
    std::vector<Int> field6;
    // pointer to deck id name
    std::vector<Long> pointer1;
    // pointer to deck name
    std::vector<Long> pointer2;
    // pointer to third string
    std::vector<Long> pointer3;
    // pointer to 4th string
    std::vector<Long> pointer4;

    // deck id name
    std::vector<std::string> string1;
    // deck name
    std::vector<std::string> string2;
    // ???
    std::vector<std::u16string> string3;
    // ???
    std::vector<std::u16string> string4;
};

Byte ReadByte(std::ifstream &file);
Short ReadShort(std::ifstream &file);
Int ReadInt(std::ifstream &file);
Long ReadLong(std::ifstream &file);
std::u16string ReadString(std::ifstream &file);
std::string ReadByteString(std::ifstream &file);

void WriteByte(std::ofstream &file, Byte value);
void WriteShort(std::ofstream &file, Short value);
void WriteInt(std::ofstream &file, Int value);
void WriteLong(std::ofstream &file, Long value);
void WriteString(std::ofstream &file, const std::u16string &string);
void WriteByteString(std::ofstream &file, const std::string &string);

bool ReadFile(const std::string &filename, FILE_DATA &fileData);
bool SaveFile(const std::string &filename, FILE_DATA &fileData);

std::string IntToHexString(const Int &value);

void ClearFileData(FILE_DATA &fileData);
std::string SimplifyString(const std::string &string);