#ifdef _WIN32
#include <Windows.h>
#include <wchar.h>
#include <direct.h>
#else
#include <sys/stat.h>
#endif // _WIN32

#include "acb.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define ACB_ENDIAN_BIG (1)
#define ACB_ENDIAN_LITTLE (0)
#define BUF_SIZE_MAX (128 * 1024)

static int get_data_type_size(int data_type);

static bool check_memory_endian() {
  uint32_t n = 1;
  uint8_t b = (uint8_t)n;
  if (b) {
    return ACB_ENDIAN_LITTLE;
  } else {
    return ACB_ENDIAN_BIG;
  }
}

static uint8_t read_uint_8(uint8_t* buf) {
  return *buf;
}

static int8_t read_int_8(uint8_t* buf) {
  return *((int8_t*)buf);
}

static uint16_t read_uint_16(uint8_t* buf, bool endian) {
  if (endian == check_memory_endian()) {
    return *((uint16_t*)buf);
  } else {
    uint8_t tmp[2] = { buf[1], buf[0] };
    return *((uint16_t*)tmp);
  }
}

static int16_t read_int_16(uint8_t* buf, bool endian) {
  if (endian == check_memory_endian()) {
    return *((int16_t*)buf);
  } else {
    uint8_t tmp[2] = { buf[1], buf[0] };
    return *((int16_t*)tmp);
  }
}

static uint32_t read_uint_32(uint8_t* buf, bool endian) {
  if (endian == check_memory_endian()) {
    return *((uint32_t*)buf);
  } else {
    uint8_t tmp[4] = { buf[3], buf[2], buf[1], buf[0] };
    return *((uint32_t*)tmp);
  }
}

static int32_t read_int_32(uint8_t* buf, bool endian) {
  if (endian == check_memory_endian()) {
    return *((int32_t*)buf);
  } else {
    uint8_t tmp[4] = { buf[3], buf[2], buf[1], buf[0] };
    return *((int32_t*)tmp);
  }
}

static int64_t read_int_64(uint8_t* buf, bool endian) {
  if (endian == check_memory_endian()) {
    return *((int64_t*)buf);
  } else {
    uint8_t tmp[8] = { buf[7], buf[6], buf[5], buf[4], buf[3], buf[2], buf[1], buf[0] };
    return *((int64_t*)tmp);
  }
}

static uint64_t read_uint_64(uint8_t* buf, bool endian) {
  if (endian == check_memory_endian()) {
    return *((uint64_t*)buf);
  } else {
    uint8_t tmp[8] = { buf[7], buf[6], buf[5], buf[4], buf[3], buf[2], buf[1], buf[0] };
    return *((uint64_t*)tmp);
  }
}

static float read_float(uint8_t* buf, bool endian) {
  if (endian == check_memory_endian()) {
    return *((float*)buf);
  } else {
    uint8_t tmp[8] = { buf[3], buf[2], buf[1], buf[0] };
    return *((float*)tmp);
  }
}

static double read_double(uint8_t* buf, bool endian) {
  if (endian == check_memory_endian()) {
    return *((double*)buf);
  } else {
    uint8_t tmp[8] = { buf[7], buf[6], buf[5], buf[4], buf[3], buf[2], buf[1], buf[0] };
    return *((double*)tmp);
  }
}

static int acb_fseek(acb* acbp, long pos) {
  if (pos > acbp->size) {
    return 1;
  }
  long origin = ftell(acbp->f);
  int res = fseek(acbp->f, acbp->start + pos, SEEK_SET);
  if (!res) {
    acbp->pos = pos;
    fseek(acbp->f, origin, SEEK_SET);
    return 0;
  }
  return res;
}

static long acb_ftell(acb* acbp) {
  return acbp->pos;
}

static size_t acb_fread(void* dest, size_t size, size_t count, acb* acbp) {
  if (acbp->pos + (size * count) > (size_t)acbp->size) {
    return 0;
  }
  long origin = ftell(acbp->f);
  fseek(acbp->f, acbp->start + acbp->pos, SEEK_SET);
  int read = fread(dest, size, count, acbp->f);
  if (read != 0) {
    acbp->pos = acbp->pos + read;
    fseek(acbp->f, origin, SEEK_SET);
    return read;
  }
  fseek(acbp->f, origin, SEEK_SET);
  return 0;
}

static int acb_fclose(acb* acbp) {
  if (acbp->f && acbp->start == 0) {
    return fclose(acbp->f);
    acbp->f = NULL;
  }

  acbp->start = 0;
  acbp->size = 0;
  acbp->pos = 0;
  return 0;
}

static void get_encode_type_suffix(uint8_t type, char* out) {
  switch (type) {
  case 0:
  {
    strcpy(out, ".adx");
    return;
  }
  case 2:
  {
    strcpy(out, ".hca");
    return;
  }
  case 7:
  {
    strcpy(out, ".at3");
    return;
  }
  case 8:
  {
    strcpy(out, ".vag");
    return;
  }
  case 9:
  {
    strcpy(out, ".bcwav");
    return;
  }
  case 13:
  {
    strcpy(out, ".dsp");
    return;
  }
  default:
    return;
  }
}

acb* acb_open(const char* path) {
#ifdef _WIN32
  int len = MultiByteToWideChar(CP_UTF8, 0, path, -1, NULL, 0);
  wchar_t* wpath = (wchar_t*)malloc(len * sizeof(wchar_t));
  if (!wpath) return NULL;
  MultiByteToWideChar(CP_UTF8, 0, path, -1, wpath, len);
  FILE* f = _wfopen(wpath, L"rb");
  free(wpath);
#else
  FILE* f = fopen(path, "rb");
#endif // _WIN32

  if (f) {
    acb* _acb = (acb*)malloc(sizeof(acb));
    if (!_acb) return NULL;
    memset(_acb, 0, sizeof(acb));

    _acb->f = f;
    memset(_acb->path, 0, MAX_PATH_LENGTH);
    strcpy(_acb->path, path);

    _acb->start = 0;
    fseek(f, 0L, SEEK_END);
    _acb->size = ftell(f);
    fseek(f, 0L, SEEK_SET);
    _acb->pos = 0;

    acb_read_file_header(_acb, &(_acb->file_header));
    acb_read_utf_header(_acb, &(_acb->utf_header));
    _acb->name = acb_read_string(_acb, _acb->utf_header.table_name_string_offset);

    _acb->columns = (acb_utf_column*)malloc(_acb->utf_header.column_length * sizeof(acb_utf_column));
    acb_read_column(_acb, _acb->columns, _acb->utf_header.column_length);

    return _acb;
  } else {
    return NULL;
  }
}

acb* acb_memopen(acb* parent, long start, long size) {
  acb* _acb = (acb*)malloc(sizeof(acb));
  if (!_acb) return NULL;
  memset(_acb, 0, sizeof(acb));

  _acb->f = parent->f;
  memset(_acb->path, 0, MAX_PATH_LENGTH);

  _acb->start = start;
  _acb->size = size;
  _acb->pos = 0;

  acb_read_file_header(_acb, &(_acb->file_header));
  acb_read_utf_header(_acb, &(_acb->utf_header));
  _acb->name = acb_read_string(_acb, _acb->utf_header.table_name_string_offset);

  _acb->columns = (acb_utf_column*)malloc(_acb->utf_header.column_length * sizeof(acb_utf_column));
  acb_read_column(_acb, _acb->columns, _acb->utf_header.column_length);

  return _acb;
}

acb* acb_memopen_awb(acb* parent, long start, long size) {
  acb* _acb = (acb*)malloc(sizeof(acb));
  if (!_acb) return NULL;
  memset(_acb, 0, sizeof(acb));

  _acb->f = parent->f;
  memset(_acb->path, 0, MAX_PATH_LENGTH);

  _acb->start = start;
  _acb->size = size;
  _acb->pos = 0;

  return _acb;
}

void acb_close(acb* acbp) {
  acb_fclose(acbp);
  acbp->f = NULL;
  acbp->start = 0;
  acbp->size = 0;
  acbp->pos = 0;

  if (acbp->name != NULL) {
    free(acbp->name);
    acbp->name = NULL;
  }

  if (acbp->columns != NULL) {
    for (int i = 0; i < acbp->utf_header.column_length; i++) {
      if (acbp->columns[i].name) { free(acbp->columns[i].name); acbp->columns[i].name = NULL; }
      if (acbp->columns[i].const_value) { free(acbp->columns[i].const_value); acbp->columns[i].const_value = NULL; }
    }
    free(acbp->columns);
    acbp->columns = NULL;
  }

  free(acbp);
}

void acb_read_file_header(acb* acbp, acb_file_header* header) {
  if (!acbp) return;
  long original_pos = acb_ftell(acbp);
  acb_fseek(acbp, 0);
  uint8_t magic[4];
  uint8_t size[4];
  acb_fread(magic, 1, 4, acbp);
  acb_fread(size, 1, 4, acbp);

  header->magic = read_uint_32(magic, ACB_ENDIAN_BIG);
  header->size = read_uint_32(size, ACB_ENDIAN_BIG);
  acb_fseek(acbp, original_pos);
}

uint32_t acb_get_size(acb* acbp) {
  if (!acbp) return 0;
  return acbp->file_header.size;
}

void acb_read_utf_header(acb* acbp, acb_utf_header* header) {
  if (!acbp) return;
  long original_pos = acb_ftell(acbp);
  acb_fseek(acbp, 8);

  uint8_t unknown1[2];
  uint8_t table_data_offset[2];
  uint8_t string_data_offset[4];
  uint8_t binary_data_offset[4];
  uint8_t table_name_string_offset[4];
  uint8_t column_length[2];
  uint8_t row_total_byte[2];
  uint8_t row_length[4];

  acb_fread(unknown1, 1, 2, acbp);
  acb_fread(table_data_offset, 1, 2, acbp);
  acb_fread(string_data_offset, 1, 4, acbp);
  acb_fread(binary_data_offset, 1, 4, acbp);
  acb_fread(table_name_string_offset, 1, 4, acbp);
  acb_fread(column_length, 1, 2, acbp);
  acb_fread(row_total_byte, 1, 2, acbp);
  acb_fread(row_length, 1, 4, acbp);

  header->unknown1 = read_uint_16(unknown1, ACB_ENDIAN_BIG);
  header->table_data_offset = read_uint_16(table_data_offset, ACB_ENDIAN_BIG);
  header->string_data_offset = read_uint_32(string_data_offset, ACB_ENDIAN_BIG);
  header->binary_data_offset = read_uint_32(binary_data_offset, ACB_ENDIAN_BIG);
  header->table_name_string_offset = read_uint_32(table_name_string_offset, ACB_ENDIAN_BIG);
  header->column_length = read_uint_16(column_length, ACB_ENDIAN_BIG);
  header->row_total_byte = read_uint_16(row_total_byte, ACB_ENDIAN_BIG);
  header->row_length = read_uint_32(row_length, ACB_ENDIAN_BIG);

  acb_fseek(acbp, original_pos);
}

char* acb_read_string(acb* acbp, uint32_t offset) {
  if (!acbp) return NULL;
  long original_pos = acb_ftell(acbp);
  acb_fseek(acbp, 8 + acbp->utf_header.string_data_offset + offset);

  char c;
  uint32_t len = 0;

  while (true) {
    acb_fread(&c, 1, 1, acbp);
    len++;
    if (c == '\0') {
      break;
    }
  }

  char* res = (char*)malloc(len * sizeof(char));
  acb_fseek(acbp, 8 + acbp->utf_header.string_data_offset + offset);
  acb_fread(res, 1, len, acbp);

  acb_fseek(acbp, original_pos);
  return res;
}

void acb_read_column(acb* acbp, acb_utf_column* columns, int column_length) {
  if (!acbp) return;
  long original_pos = acb_ftell(acbp);

  acb_fseek(acbp, 8 + 24);

  for (int i = 0; i < column_length; i++) {
    uint8_t type[1];
    uint8_t name_offset[4];
    acb_fread(type, 1, 1, acbp);
    acb_fread(name_offset, 1, 4, acbp);

    columns[i].type = type[0];
    columns[i].index = i;
    columns[i].name = acb_read_string(acbp, read_uint_32(name_offset, ACB_ENDIAN_BIG));
    columns[i].const_value = NULL;

    int column_type = columns[i].type & 0xf0;
    int data_type = columns[i].type & 0x0f;

    if (column_type == 0x30 || column_type == 0x70) {
      switch (data_type) {
      case 0x00:
      {
        uint8_t* data = (uint8_t*)malloc(1);
        acb_fread(data, 1, 1, acbp);
        columns[i].const_value = data;
        break;
      }
      case 0x01:
      {
        int8_t* data = (int8_t*)malloc(1);
        acb_fread(data, 1, 1, acbp);
        columns[i].const_value = data;
        break;
      }
      case 0x02:
      {
        uint8_t data[2];
        acb_fread(data, 1, 2, acbp);
        uint16_t* res = (uint16_t*)malloc(2);
        if (!res) break;
        *res = read_uint_16(data, ACB_ENDIAN_BIG);
        columns[i].const_value = res;
        break;
      }
      case 0x03:
      {
        uint8_t data[2];
        acb_fread(data, 1, 2, acbp);
        int16_t* res = (int16_t*)malloc(2);
        if (!res) break;
        *res = read_int_16(data, ACB_ENDIAN_BIG);
        columns[i].const_value = res;
        break;
      }
      case 0x04:
      {
        uint8_t data[4];
        acb_fread(data, 1, 4, acbp);
        uint32_t* res = (uint32_t*)malloc(4);
        if (!res) break;
        *res = read_uint_32(data, ACB_ENDIAN_BIG);
        columns[i].const_value = res;
        break;
      }
      case 0x05:
      {
        uint8_t data[4];
        acb_fread(data, 1, 4, acbp);
        int32_t* res = (int32_t*)malloc(4);
        if (!res) break;
        *res = read_int_32(data, ACB_ENDIAN_BIG);
        columns[i].const_value = res;
        break;
      }
      case 0x08:
      {
        uint8_t data[4];
        acb_fread(data, 1, 4, acbp);
        float* res = (float*)malloc(4);
        if (!res) break;
        *res = read_float(data, ACB_ENDIAN_BIG);
        columns[i].const_value = res;
        break;
      }
      case 0x06:
      {
        uint8_t data[8];
        acb_fread(data, 1, 8, acbp);
        uint64_t* res = (uint64_t*)malloc(8);
        if (!res) break;
        *res = read_uint_64(data, ACB_ENDIAN_BIG);
        columns[i].const_value = res;
        break;
      }
      case 0x07:
      {
        uint8_t data[8];
        acb_fread(data, 1, 8, acbp);
        int64_t* res = (int64_t*)malloc(8);
        if (!res) break;
        *res = read_int_64(data, ACB_ENDIAN_BIG);
        columns[i].const_value = res;
        break;
      }
      case 0x09:
      {
        uint8_t data[8];
        acb_fread(data, 1, 8, acbp);
        double* res = (double*)malloc(8);
        if (!res) break;
        *res = read_double(data, ACB_ENDIAN_BIG);
        columns[i].const_value = res;
        break;
      }
      case 0x0A:
      {
        uint8_t data[4];
        acb_fread(data, 1, 4, acbp);
        columns[i].const_value = acb_read_string(acbp, read_uint_32(data, ACB_ENDIAN_BIG));
        break;
      }
      case 0x0B:
      {
        uint8_t offset[4];
        uint8_t length[4];

        acb_fread(offset, 1, 4, acbp);
        acb_fread(length, 1, 4, acbp);

        acb_utf_binary_desc* desc = (acb_utf_binary_desc*)malloc(sizeof(acb_utf_binary_desc));
        if (!desc) break;
        desc->offset = read_uint_32(offset, ACB_ENDIAN_BIG);
        desc->length = read_uint_32(length, ACB_ENDIAN_BIG);
        columns[i].const_value = desc;
        break;
      }
      default:
        break;
      }
    }
  }

  acb_fseek(acbp, original_pos);
}

int get_data_type_size(int data_type) {
  switch (data_type) {
  case 0x00: { return 1; }
  case 0x01: { return 1; }
  case 0x02: { return 2; }
  case 0x03: { return 2; }
  case 0x04: { return 4; }
  case 0x05: { return 4; }
  case 0x06: { return 8; }
  case 0x07: { return 8; }
  case 0x08: { return 4; }
  case 0x09: { return 8; }
  case 0x0A: { return 4; }
  case 0x0B: { return 8; }
  default: { return 0; }
  }
}

int acb_get_row_data(acb* acbp, uint32_t row_index, const char* column_name, void* res, uint32_t buf_size) {
  if (!acbp) return 0;

  int column_index = -1;
  int i = 0;
  for (i = 0; i < acbp->utf_header.column_length; i++) {
    if (strcmp(acbp->columns[i].name, column_name) == 0) {
      column_index = i;
      break;
    }
  }

  if (column_index == -1) return 0;

  int data_type = acbp->columns[column_index].type & 0x0f;

  if (res == NULL) {
    return data_type;
  }

  int column_type = acbp->columns[column_index].type & 0xf0;

  if ((column_type == 0x30 || column_type == 0x70) && acbp->columns[column_index].const_value != NULL) {
    switch (data_type) {
    case 0x00: { *((uint8_t*)res) = *((uint8_t*)(acbp->columns[column_index].const_value)); return data_type; }
    case 0x01: { *((int8_t*)res) = *((int8_t*)(acbp->columns[column_index].const_value)); return data_type; }
    case 0x02: { *((uint16_t*)res) = *((uint16_t*)(acbp->columns[column_index].const_value)); return data_type; }
    case 0x03: { *((int16_t*)res) = *((int16_t*)(acbp->columns[column_index].const_value)); return data_type; }
    case 0x04: { *((uint32_t*)res) = *((uint32_t*)(acbp->columns[column_index].const_value)); return data_type; }
    case 0x05: { *((int32_t*)res) = *((int32_t*)(acbp->columns[column_index].const_value)); return data_type; }
    case 0x06: { *((uint64_t*)res) = *((uint64_t*)(acbp->columns[column_index].const_value)); return data_type; }
    case 0x07: { *((int64_t*)res) = *((int64_t*)(acbp->columns[column_index].const_value)); return data_type; }
    case 0x08: { *((float*)res) = *((float*)(acbp->columns[column_index].const_value)); return data_type; }
    case 0x09: { *((double*)res) = *((double*)(acbp->columns[column_index].const_value)); return data_type; }
    case 0x0A:
    {
      if (strlen((char*)(acbp->columns[column_index].const_value)) > buf_size - 1) {
        strncpy((char*)res, (char*)(acbp->columns[column_index].const_value), buf_size - 1);
      } else {
        strcpy((char*)res, (char*)(acbp->columns[column_index].const_value));
      }
      return data_type;
    }
    case 0x0B:
    {
      ((acb_utf_binary_desc*)res)->offset = ((acb_utf_binary_desc*)acbp->columns[column_index].const_value)->offset;
      ((acb_utf_binary_desc*)res)->length = ((acb_utf_binary_desc*)acbp->columns[column_index].const_value)->length;
      return data_type;
    }
    default: { return 0; }
    }
  }

  long original_pos = acb_ftell(acbp);

  uint32_t pos = 8 + acbp->utf_header.table_data_offset + row_index * acbp->utf_header.row_total_byte;

  for (i = 0; i < column_index; i++) {
    int column_type = acbp->columns[i].type & 0xf0;
    if (column_type == 0x30 || column_type == 0x70) {
      continue;
    }
    int data_type = acbp->columns[i].type & 0x0f;
    pos += get_data_type_size(data_type);
  }

  acb_fseek(acbp, pos);

  switch (data_type) {
  case 0x00: { uint8_t tmp[1]; acb_fread(tmp, 1, 1, acbp); *((uint8_t*)res) = read_uint_8(tmp); break; }
  case 0x01: { uint8_t tmp[1]; acb_fread(tmp, 1, 1, acbp); *((int8_t*)res) = read_int_8(tmp); break; }
  case 0x02: { uint8_t tmp[2]; acb_fread(tmp, 1, 2, acbp); *((uint16_t*)res) = read_uint_16(tmp, ACB_ENDIAN_BIG); break; }
  case 0x03: { uint8_t tmp[2]; acb_fread(tmp, 1, 2, acbp); *((int16_t*)res) = read_int_16(tmp, ACB_ENDIAN_BIG); break; }
  case 0x04: { uint8_t tmp[4]; acb_fread(tmp, 1, 4, acbp); *((uint32_t*)res) = read_uint_32(tmp, ACB_ENDIAN_BIG); break; }
  case 0x05: { uint8_t tmp[4]; acb_fread(tmp, 1, 4, acbp); *((int32_t*)res) = read_int_32(tmp, ACB_ENDIAN_BIG); break; }
  case 0x06: { uint8_t tmp[8]; acb_fread(tmp, 1, 8, acbp); *((uint64_t*)res) = read_uint_64(tmp, ACB_ENDIAN_BIG); break; }
  case 0x07: { uint8_t tmp[8]; acb_fread(tmp, 1, 8, acbp); *((int64_t*)res) = read_int_64(tmp, ACB_ENDIAN_BIG); break; }
  case 0x08: { uint8_t tmp[4]; acb_fread(tmp, 1, 4, acbp); *((float*)res) = read_float(tmp, ACB_ENDIAN_BIG); break; }
  case 0x09: { uint8_t tmp[8]; acb_fread(tmp, 1, 8, acbp); *((double*)res) = read_double(tmp, ACB_ENDIAN_BIG); break; }
  case 0x0A:
  {
    uint8_t tmp[4];
    acb_fread(tmp, 1, 4, acbp);
    char* tmpstr = acb_read_string(acbp, read_uint_32(tmp, ACB_ENDIAN_BIG));
    if (strlen(tmpstr) > buf_size - 1) {
      strncpy((char*)res, tmpstr, buf_size - 1);
    } else {
      strcpy((char*)res, tmpstr);
    }
    free(tmpstr);
    break;
  }
  case 0x0B:
  {
    uint8_t offset[4];
    uint8_t length[4];

    acb_fread(offset, 1, 4, acbp);
    acb_fread(length, 1, 4, acbp);
    ((acb_utf_binary_desc*)res)->offset = read_uint_32(offset, ACB_ENDIAN_BIG);
    ((acb_utf_binary_desc*)res)->length = read_uint_32(length, ACB_ENDIAN_BIG);
    break;
  }
  default: { break; }
  }

  acb_fseek(acbp, original_pos);
  return data_type;
}

acb_track* acb_get_track_list(acb* acbp, uint32_t* len) {
  if (!acbp) return NULL;
  if (acbp->start != 0) return NULL;

  acb_utf_binary_desc desc_cue_table;
  acb_utf_binary_desc desc_cue_name_table;
  acb_utf_binary_desc desc_wave_form_table;
  acb_utf_binary_desc desc_synth_table;

  acb_get_row_data(acbp, 0, "CueTable", &desc_cue_table, 0);
  acb_get_row_data(acbp, 0, "CueNameTable", &desc_cue_name_table, 0);
  acb_get_row_data(acbp, 0, "WaveformTable", &desc_wave_form_table, 0);
  acb_get_row_data(acbp, 0, "SynthTable", &desc_synth_table, 0);

  acb* cue_table = acb_memopen(acbp, 8 + acbp->utf_header.binary_data_offset + desc_cue_table.offset, desc_cue_table.length);
  acb* name_table = acb_memopen(acbp, 8 + acbp->utf_header.binary_data_offset + desc_cue_name_table.offset, desc_cue_name_table.length);
  acb* wave_form_table = acb_memopen(acbp, 8 + acbp->utf_header.binary_data_offset + desc_wave_form_table.offset, desc_wave_form_table.length);
  acb* synth_table = acb_memopen(acbp, 8 + acbp->utf_header.binary_data_offset + desc_synth_table.offset, desc_synth_table.length);

  acb_track* track_list = (acb_track*)malloc(cue_table->utf_header.row_length * sizeof(acb_track));
  if (track_list) {
    memset(track_list, 0, cue_table->utf_header.row_length * sizeof(acb_track));
    for (uint16_t i = 0; i < cue_table->utf_header.row_length; i++) {
      int found = 0;
      for (uint16_t j = 0; j < name_table->utf_header.row_length; j++) {
        uint16_t cue_index = 0;
        acb_get_row_data(name_table, j, "CueIndex", &cue_index, 0);
        if (cue_index == i) {
          acb_get_row_data(name_table, j, "CueName", (track_list + i)->cue_name, 64);
          found = 1;
        }
      }

      if (!found) {
        strcpy((track_list + i)->cue_name, "UNKNOWN");
      }

      uint32_t cue_id = 0;
      acb_get_row_data(cue_table, i, "CueId", &cue_id, 0);

      (track_list + i)->cue_id = cue_id;

      uint8_t reference_type = 0;
      acb_get_row_data(cue_table, i, "ReferenceType", &reference_type, 0);
      if (reference_type != 3 && reference_type != 8) {
        printf("ReferenceType %d not implemented.\n", reference_type);
        break;
      }

      uint16_t reference_index = 0;
      acb_get_row_data(cue_table, i, "ReferenceIndex", &reference_index, 0);

      acb_utf_binary_desc desc_ref_item;
      acb_get_row_data(synth_table, reference_index, "ReferenceItems", &desc_ref_item, 0);


      uint8_t _b[2];
      long origin = acb_ftell(synth_table);
      acb_fseek(synth_table, 8 + synth_table->utf_header.binary_data_offset + desc_ref_item.offset + 2);
      acb_fread(_b, 1, 2, synth_table);
      acb_fseek(synth_table, origin);
      uint16_t b = read_uint_16(_b, ACB_ENDIAN_BIG);

      uint16_t wav_id = 0;
      // if (b >= wave_form_table->utf_header.row_length) {
      if (b < wave_form_table->utf_header.row_length) {
        int res = acb_get_row_data(wave_form_table, b, "Id", &wav_id, 0);
        if (res == 0) {
          acb_get_row_data(wave_form_table, b, "MemoryAwbId", &wav_id, 0);
        }
      } else {
        acb_get_row_data(wave_form_table, b, "MemoryAwbId", &wav_id, 0);
      }

      (track_list + i)->wav_id = wav_id;

      uint8_t encode_type = 0;
      uint8_t streaming = 0;
      acb_get_row_data(wave_form_table, b, "EncodeType", &encode_type, 0);
      acb_get_row_data(wave_form_table, b, "Streaming", &streaming, 0);

      (track_list + i)->encode_type = encode_type;
      (track_list + i)->streaming = streaming;
    }
  }

  *len = cue_table->utf_header.row_length;

  acb_close(cue_table);
  acb_close(name_table);
  acb_close(wave_form_table);
  acb_close(synth_table);

  return track_list;
}


int acb_extract(acb* acbp, const char* target_dir, acb_extract_callback callback) {
  if (!acbp) return 1;
  if (acbp->start != 0) return 2;

  uint32_t track_list_length = 0;
  acb_track* track_list = acb_get_track_list(acbp, &track_list_length);

  acb_utf_binary_desc desc_awb;
  acb_get_row_data(acbp, 0, "AwbFile", &desc_awb, 0);

  acb* awb = acb_memopen_awb(acbp, 8 + acbp->utf_header.binary_data_offset + desc_awb.offset, desc_awb.length);

  uint8_t magic[4];
  acb_fread(magic, 1, 4, awb);
  if (read_uint_32(magic, ACB_ENDIAN_BIG) != 0x41465332) {
    acb_close(awb);
    free(track_list);
    return 3;
  }

  uint8_t offset_size = 0;
  acb_fseek(awb, 5);
  acb_fread(&offset_size, 1, 1, awb);
  acb_fseek(awb, 8);
  uint8_t file_count_buf[4];
  uint8_t alignment_buf[4];
  acb_fread(file_count_buf, 1, 4, awb);
  acb_fread(alignment_buf, 1, 4, awb);
  uint32_t file_count = read_uint_32(file_count_buf, ACB_ENDIAN_LITTLE);
  uint32_t alignment = read_uint_32(alignment_buf, ACB_ENDIAN_LITTLE);

  uint16_t* id_arr = (uint16_t*)malloc(file_count * sizeof(uint16_t));
  if (!id_arr) {
    acb_close(awb);
    free(track_list);
    return 4;
  }

  uint8_t id_buf[2];
  for (uint32_t i = 0; i < file_count; i++) {
    acb_fread(id_buf, 1, 2, awb);
    *(id_arr + i) = read_uint_16(id_buf, ACB_ENDIAN_LITTLE);
  }

  uint64_t* file_end_points = (uint64_t*)malloc((file_count + 1) * sizeof(uint64_t));
  if (!file_end_points) {
    free(id_arr);
    acb_close(awb);
    free(track_list);
    return 4;
  }

  uint8_t ep_8[8] = { 0 };
  for (uint32_t i = 0; i < file_count + 1; i++) {
    acb_fread(ep_8, 1, offset_size, awb);
    *(file_end_points + i) = read_uint_64(ep_8, ACB_ENDIAN_LITTLE);
  }

  acb_awb_wav_file* files = (acb_awb_wav_file*)malloc(file_count * sizeof(acb_awb_wav_file));
  if (!files) {
    free(file_end_points);
    free(id_arr);
    acb_close(awb);
    free(track_list);
    return 5;
  }

  for (uint32_t i = 0; i < file_count; i++) {
    (files + i)->id = *(id_arr + i);
    (files + i)->start = (uint32_t)ceil((double)(file_end_points[i]) / (double)(alignment)) * alignment;
    (files + i)->length = (uint32_t)(*(file_end_points + i + 1)) - (files + i)->start;
  }

  char name[MAX_PATH_LENGTH] = { 0 };
  uint8_t* buffer = (uint8_t*)malloc(BUF_SIZE_MAX * sizeof(uint8_t));
  if (!buffer) {
    free(files);
    free(file_end_points);
    free(id_arr);
    acb_close(awb);
    free(track_list);
    return 6;
  }

#ifdef _WIN32
  wchar_t* target_dir_w = NULL;
  if (target_dir) {
    int len = MultiByteToWideChar(CP_UTF8, 0, target_dir, -1, NULL, 0);
    target_dir_w = (wchar_t*)malloc(len * sizeof(wchar_t));
    if (target_dir_w) {
      MultiByteToWideChar(CP_UTF8, 0, target_dir, -1, target_dir_w, len);
      _wmkdir(target_dir_w);
      free(target_dir_w);
    }
  }
#else
  mkdir(target_dir, 0777);
#endif

  for (uint32_t i = 0; i < track_list_length; i++) {
    acb_track* track = (track_list + i);
    bool result = false;
    for (uint32_t j = 0; j < file_count; j++) {
      if ((files + j)->id == track->wav_id) {
        acb_awb_wav_file* file = (files + track->wav_id);
        uint32_t total = 0;
        uint32_t read = 0;
        acb_fseek(awb, file->start);
        memset(name, 0, MAX_PATH_LENGTH);
        if (target_dir) {
          strcat(name, target_dir);
#ifdef _WIN32
          strcat(name, "\\");
#else
          strcat(name, "/");
#endif
        }
        strcat(name, track->cue_name);
        char suffix[16] = { 0 };
        get_encode_type_suffix(track->encode_type, suffix);
        strcat(name, suffix);
#ifdef _WIN32
        int len = MultiByteToWideChar(CP_UTF8, 0, name, -1, NULL, 0);
        wchar_t* wpath = (wchar_t*)malloc(len * sizeof(wchar_t));
        if (!wpath) break;
        MultiByteToWideChar(CP_UTF8, 0, name, -1, wpath, len);
        FILE* wp = _wfopen(wpath, L"wb+");
        free(wpath);
#else
        FILE* wp = fopen(name, "wb+");
#endif
        if (!wp) {
          free(files);
          free(file_end_points);
          free(id_arr);
          acb_close(awb);
          free(track_list);
          return 7;
        }
        while (1) {
          if (total + BUF_SIZE_MAX <= file->length) {
            read = acb_fread(buffer, 1, BUF_SIZE_MAX, awb);
            total += read;
            fwrite(buffer, 1, read, wp);
          } else {
            read = acb_fread(buffer, 1, file->length - total, awb);
            total += read;
            fwrite(buffer, 1, read, wp);
            break;
          }
        }

        fclose(wp);
        result = true;
        break;
      }
    }

    if (callback) {
      callback(i + 1, track_list_length, name, result);
    }
  }

  free(buffer);
  free(files);
  free(file_end_points);
  free(id_arr);
  acb_close(awb);
  free(track_list);

  return 0;
}
