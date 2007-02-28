require 'rbconfig'

C_FILE = 'tmp.c'
EXE_FILE = 'tmp_run'
OUT_FLAG = Config::MAKEFILE_CONFIG['OUTFLAG']
CC = Config::MAKEFILE_CONFIG['CC']
STD_HEADERS = <<SRC
#include <stdlib.h>
#include <stdio.h>
SRC

def compile(text)
  File.open(C_FILE, 'w') {|f| f.write(text)}
  system("#{CC} #{C_FILE} #{OUT_FLAG} #{EXE_FILE}")
end

def capture(text)
  if not compile(text)
    raise "unexpected compile error:\n#{text}"
  end
  IO.popen(EXE_FILE) {|io| return io.read }
end

def try_capture(text)
  if compile(text)
    IO.popen(EXE_FILE) {|io| return io.read }
  end
end

def std_c_file(code)
  <<SRC
#{STD_HEADERS}
int main(int argc, char *argv[])
{
  #{code}
  return 0;
}
SRC
end
INT_SIZES_SRC = std_c_file(%s{printf("%d %d %d", sizeof(short), sizeof(int), sizeof(long));})

LONG_LONG_SIZE_SRC = std_c_file(%s{printf("%d", sizeof(long long));})

I64_SIZE_SRC = std_c_file(%s{printf("%d", sizeof(__int64));})

short_sz, int_sz, long_sz = capture(INT_SIZES_SRC).split(' ').map{|s| s.to_i}
long_long_sz = try_capture(LONG_LONG_SIZE_SRC).to_i
i64_sz = try_capture(I64_SIZE_SRC).to_i

def def_ints(type, bits)
  <<SRC
typedef #{type} f_i#{bits};
typedef unsigned #{type} f_u#{bits};
SRC
end

output = <<SRC
#ifndef false
# define false 0
#endif
#ifndef true
# define true 1
#endif

typedef unsigned int bool;
typedef unsigned char uchar;
SRC

if (short_sz == 2)
  output << def_ints('short', 16)
else
  raise "No 16-bit data type"
end

if (int_sz == 4)
  output << def_ints('int', 32)
elsif (long_sz == 4)
  output << def_ints('long', 32)
else
  raise "No 32-bit data type"
end

if (long_sz == 8)
  u64_t = 'long'
elsif (long_long_sz == 8)
  u64_t = 'long long'
elsif (i64_sz == 8)
  u64_t = '__int64'
else
  raise "No 64-bit data type"
end
output << def_ints(u64_t, 64)

["ll", "l", "L", "q", "I64"].each do |prefix|
  src = std_c_file(<<SRC)
    printf("%#{prefix}u", (unsigned #{u64_t})18446744073709551615);
SRC
  if try_capture(src).to_i == 18446744073709551615
    output << <<SRC
#define I64P "#{prefix}d"
#define U64P "#{prefix}u"
SRC
    break
  end
end

ISO_VMAC = <<SRC
#{STD_HEADERS}
#define ISO_TEST(fmt, ...) printf(fmt, __VA_ARGS__)
int main() {
    ISO_TEST("%d %d", 1, 1);
    return 0;
}
SRC

GNUC_VMAC = <<SRC
#{STD_HEADERS}
#define GNU_TEST(fmt, ...) printf(fmt, args...)
int main() {
    GNU_TEST("%d %d", 1, 1);
    return 0;
}
SRC

if try_capture(ISO_VMAC) == "1 1"
  output << <<SRC
#define FRT_HAS_ISO_VARARGS
#define FRT_HAS_VARARGS
SRC
elsif try_capture(GNU_VMAC) == "1 1"
  output << <<SRC
#define FRT_HAS_GNUC_VARARGS
#define FRT_HAS_VARARGS
SRC
end

ISO_FUNC = std_c_file(%s{printf("%s", __func__);})
GNUC_FUNC = std_c_file(%s{printf("%s", __FUNCTION__);})
if try_capture(ISO_FUNC) == "main"
  output << <<SRC
#define FRT_FUNC __func__
SRC
elsif try_capture(GNUC_FUNC) == "main"
  output << <<SRC
#define FRT_FUNC __FUNCTION__
SRC
else
  output << <<SRC
#define FRT_FUNC "Unknown Function"
SRC
end

puts output
