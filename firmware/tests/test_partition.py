from pathlib import Path
import struct,unittest
ROOT=Path(__file__).resolve().parents[1]
class Partitions(unittest.TestCase):
 def test_slots(self):
  rows=[]
  for line in (ROOT/'ScheduledBlind/partitions.csv').read_text().splitlines():
   if line and not line.startswith('#'):
    name,kind,sub,offset,size,*_=line.split(',');rows.append((name,int(offset,0),int(size,0)))
  self.assertEqual(rows,[('nvs',0x9000,0x5000),('otadata',0xe000,0x2000),('app0',0x10000,0x1e0000),('app1',0x1f0000,0x1e0000),('spiffs',0x3d0000,0x20000),('coredump',0x3f0000,0x10000)])
  for a,b in zip(rows,rows[1:]):self.assertLessEqual(a[1]+a[2],b[1])
  self.assertEqual(rows[-1][1]+rows[-1][2],0x400000)
 def test_emitted_table(self):
  f=ROOT/'build/ScheduledBlind.ino.partitions.bin'
  if not f.exists():self.skipTest('Compile first to inspect emitted binary')
  data=f.read_bytes();slots={}
  for off in range(0,len(data),32):
   magic,kind,sub,addr,size,label,flags=struct.unpack('<HBBII16sI',data[off:off+32])
   if magic==0x50aa:slots[label.split(bytes([0]))[0].decode()]=(addr,size)
  self.assertEqual(slots['nvs'],(0x9000,0x5000));self.assertEqual(slots['app0'],(0x10000,0x1e0000));self.assertEqual(slots['app1'],(0x1f0000,0x1e0000))
if __name__=='__main__':unittest.main()
