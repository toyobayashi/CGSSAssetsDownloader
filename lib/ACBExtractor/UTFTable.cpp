#include "./include/UTFTable.h"

unsigned int UTFTable::dataLength[12] = {
  1, 1, 2, 2, 4, 4, 8, 8, 4, 8, 4, 8
};

UTFTable::acbColumnType UTFTable::columnType = {
  0x10, 0x30, 0x50, 0x70
};

//std::string UTFTable::dataType[12] = {
//  "UInt8", // 00
//  "Int8", // 01
//  "UInt16", // 02
//  "Int16", // 03
//  "UInt32", // 04
//  "Int32", // 05
//  "UInt64", // 06
//  "Int64", // 07
//  "Float", // 08
//  "Double", // 09
//  "String", // 0a
//  "Binary" // 0b
//};

UTFTable::UTFTable(std::string acb) {
  r = new Reader;
  r->open(acb, std::ios::binary);
  if (!r->is_open()) {
    r->close();
    delete r;
    throw "Open file failed.";
  }
  unsigned int magic = r->readUInt32BE();

  if (magic != 0x40555446) {
    r->close();
    delete r;
    throw "Not acb file.";
  }
  length = r->readUInt32BE();
  if (length != r->length - 8) {
    r->close();
    delete r;
    throw "UTFTable Error length.";
  }

  readHeader();
  name = readString(header->tableNameStringOffset, nullptr);
  readColumns();
  readRows();
  
}

UTFTable::UTFTable(unsigned char* acb, unsigned int l) {
  r = new Reader(acb, l);
  unsigned int magic = r->readUInt32BE();

  if (magic != 0x40555446) {
    r->close();
    delete r;
    throw "Not acb file.";
  }
  length = r->readUInt32BE();
  if (length != r->length - 8) {
    r->close();
    delete r;
    throw "UTFTable Error length.";
  }

  readHeader();
  name = readString(header->tableNameStringOffset, nullptr);
  readColumns();
  readRows();

}

UTFTable::~UTFTable() {
  r->close();
  delete r;
  delete name;
  for (unsigned int i = 0; i < header->columnLength; i++) {
    delete[] columns[i].columnName;
    if (columns[i].columnType == UTFTable::columnType.CONSTANT || columns[i].columnType == UTFTable::columnType.CONSTANT2) {
      deleteVoid(columns[i]);
    }
  }

  for (unsigned int i = 0; i < header->rowLength; i++) {
    for (unsigned int j = 0; j < header->columnLength; j++) {
      deleteVoid(rows[i][j]);
    }
    delete[] rows[i];
  }
  delete header;
  delete[] columns;
  delete[] rows;
}

acbData* UTFTable::get(unsigned int lineNumber, std::string columnName) {
  if (lineNumber >= header->rowLength) return nullptr;
  for (unsigned int i = 0; i < header->columnLength; i++) {
    if (rows[lineNumber][i].columnName == columnName) {
      return &(rows[lineNumber][i]);
    }
  }
  return nullptr;
}

void UTFTable::readHeader() {
  std::streampos back = r->tell();
  r->seek(8 + 0);
  header = new acbHeader;
  
  header->u1 = r->readUInt16BE();
  header->tableDataOffset = r->readUInt16BE();
  header->stringDataOffset = r->readUInt32BE();
  header->binaryDataOffset = r->readUInt32BE();
  header->tableNameStringOffset = r->readUInt32BE();
  header->columnLength = r->readUInt16BE();
  header->rowTotalByte = r->readUInt16BE();
  header->rowLength = r->readUInt32BE();
  r->seek(back);
}

char* UTFTable::readString(std::streampos offset, unsigned int* outlength) {
  std::streampos back = r->tell();
  r->seek(header->stringDataOffset + 8, offset);
  unsigned int length = 0;
  while (true) {
    length++;
    if (r->readUInt8() == 0) break;
  }
  r->seek(header->stringDataOffset + 8, offset);
  if (outlength) *outlength = length - 1;
  char* outstr = new char[length];
  r->read(outstr, length);
  r->seek(back);

  return outstr;
}

void UTFTable::readBinary(unsigned char* outbuf, std::streampos offset, std::streampos length) {
  std::streampos back = r->tell();
  r->seek(header->binaryDataOffset + 8, offset);
  r->read((char*)outbuf, length);
  r->seek(back);
}

void UTFTable::readColumns() {
  std::streampos back = r->tell();
  r->seek(24 + 8);
  columns = new acbColumn[header->columnLength];

  for (unsigned int i = 0; i < header->columnLength; i++) {
    unsigned char columnTypeAndDataType = r->readUInt8();
    unsigned int nameOffset = r->readUInt32BE();
    unsigned char columnType = columnTypeAndDataType & 0xf0;
    unsigned char dataType = columnTypeAndDataType & 0x0f;

    char* columnName = readString(nameOffset, nullptr);

    columns[i].columnName = columnName;
    columns[i].dataType = dataType;
    columns[i].columnType = columnType;

    if (columnType == UTFTable::columnType.CONSTANT || columnType == UTFTable::columnType.CONSTANT2) {
      readData(dataType, columns[i]);
    }
  }
  r->seek(back);
}

void UTFTable::readRows() {

  rows = new acbRow[header->rowLength];

  for (unsigned int ri = 0; ri < header->rowLength; ri++) {
    rows[ri] = new acbData[header->columnLength];
    unsigned int dataPos = ri * header->rowTotalByte;

    for (unsigned int i = 0; i < header->columnLength; i++) {
      rows[ri][i].columnName = columns[i].columnName;
      rows[ri][i].dataType = columns[i].dataType;
      if (columns[i].columnType == UTFTable::columnType.CONSTANT || columns[i].columnType == UTFTable::columnType.CONSTANT2) {
        unsigned char* data = new unsigned char[columns[i].length];
        for (unsigned int j = 0; j < columns[i].length; j++) {
          data[j] = static_cast<unsigned char*>(columns[i].data)[j];
        }
        rows[ri][i].data = static_cast<void*>(data);
        rows[ri][i].length = columns[i].length;
      } else {

        std::streampos back = r->tell();
        r->seek(header->tableDataOffset + 8, dataPos);

        readData(rows[ri][i].dataType, rows[ri][i]);
        r->seek(back);
        dataPos += UTFTable::dataLength[columns[i].dataType];
      }
    }
  }
}

template<typename T, typename Arg>
void UTFTable::readDataTemplate(Arg& columnOrData) {
  T* data = new T;
  *data = r->readByte<T>();
  columnOrData.data = static_cast<void*>(data);
  columnOrData.length = sizeof(T);
}

template<typename Arg>
void UTFTable::deleteVoid(Arg& columnOrData) {
  if (columnOrData.dataType == static_cast<unsigned char>(0x0b))
    delete[] static_cast<unsigned int*>(columnOrData.data);
  else if (columnOrData.dataType == static_cast<unsigned char>(0x0a))
    delete[] static_cast<char*>(columnOrData.data);
  else if (columnOrData.dataType == static_cast<unsigned char>(0x00))
    delete static_cast<unsigned char*>(columnOrData.data);
  else if (columnOrData.dataType == static_cast<unsigned char>(0x01))
    delete static_cast<char*>(columnOrData.data);
  else if (columnOrData.dataType == static_cast<unsigned char>(0x02))
    delete static_cast<unsigned short*>(columnOrData.data);
  else if (columnOrData.dataType == static_cast<unsigned char>(0x03))
    delete static_cast<short*>(columnOrData.data);
  else if (columnOrData.dataType == static_cast<unsigned char>(0x04))
    delete static_cast<unsigned int*>(columnOrData.data);
  else if (columnOrData.dataType == static_cast<unsigned char>(0x05))
    delete static_cast<int*>(columnOrData.data);
  else if (columnOrData.dataType == static_cast<unsigned char>(0x06))
    delete static_cast<unsigned long long*>(columnOrData.data);
  else if (columnOrData.dataType == static_cast<unsigned char>(0x07))
    delete static_cast<long long*>(columnOrData.data);
  else if (columnOrData.dataType == static_cast<unsigned char>(0x08))
    delete static_cast<float*>(columnOrData.data);
  else if (columnOrData.dataType == static_cast<unsigned char>(0x09))
    delete static_cast<double*>(columnOrData.data);
}

template<typename Arg>
void UTFTable::readData(unsigned char type, Arg& columnOrData) {
  switch (type) {
  case 0x00: {
    readDataTemplate<unsigned char, Arg>(columnOrData);
    break;
  }

  case 0x01: {
    readDataTemplate<char, Arg>(columnOrData);
    break;
  }

  case 0x02: {
    readDataTemplate<unsigned short, Arg>(columnOrData);
    break;
  }

  case 0x03: {
    readDataTemplate<short, Arg>(columnOrData);
    break;
  }

  case 0x04: {
    readDataTemplate<unsigned int, Arg>(columnOrData);
    break;
  }

  case 0x05: {
    readDataTemplate<int, Arg>(columnOrData);
    break;
  }

  case 0x06: {
    readDataTemplate<unsigned long long, Arg>(columnOrData);
    break;
  }

  case 0x07: {
    readDataTemplate<long long, Arg>(columnOrData);
    break;
  }

  case 0x08: {
    readDataTemplate<float, Arg>(columnOrData);
    break;
  }

  case 0x09: {
    readDataTemplate<double, Arg>(columnOrData);
    break;
  }

  case 0x0a: {
    unsigned int l = 0;
    char* data = readString(r->readUInt32BE(), &l);
    columnOrData.length = l;
    columnOrData.data = static_cast<void*>(data);
    break;
  }
  case 0x0b: {
    unsigned int* data = new unsigned int[2];
    data[0] = r->readUInt32BE(); // offset
    data[1] = r->readUInt32BE(); // length
    // readBinary(data, offset, length);
    columnOrData.length = data[1];
    columnOrData.data = static_cast<void*>(data);
    break;
  }

  default: break;
  }
}
