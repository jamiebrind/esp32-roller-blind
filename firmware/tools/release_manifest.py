from pathlib import Path
import argparse,json,hashlib
parser=argparse.ArgumentParser();parser.add_argument('--binary',required=True);parser.add_argument('--version',required=True);parser.add_argument('--output',required=True);args=parser.parse_args()
b=Path(args.binary).read_bytes()
if len(b)<288 or len(b)>1966080 or b[0]!=0xe9 or int.from_bytes(b[12:14],'little')!=5 or b[32:36]!=bytes.fromhex('3254cdab'):raise SystemExit('Not a compatible ESP32-C3 application image')
Path(args.output).write_text(json.dumps(dict(hardware='esp32c3-drv8871',version=args.version,settingsSchema=1,length=len(b),sha256=hashlib.sha256(b).hexdigest()),indent=2)+'\n')
print('Application manifest saved')
