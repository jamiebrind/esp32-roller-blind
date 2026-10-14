"""Generate vector SVG/PDF wiring drawings; optional PNG previews with pypdfium2.
Requires reportlab. Run from any directory: python hardware/generate_wiring.py.
Colours of additional hookup wires are recommendations, not measurements.
"""
from pathlib import Path
from html import escape
import csv
from reportlab.pdfgen import canvas
from reportlab.lib.colors import toColor
from reportlab.lib.pagesizes import A3, landscape

ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'docs/images'
OUT.mkdir(parents=True,exist_ok=True)
PDF=ROOT/'docs/Blind_Wiring_Drawings.pdf'
W,H=1680,1188
page=landscape(A3)
c=canvas.Canvas(str(PDF),pagesize=page)
c.setTitle('ESP32 Roller Blind - Terminal Wiring Drawings')
c.setAuthor('jamiebrind')
C={'red':'#c62828','black':'#263238','blue':'#1565c0','violet':'#7b1fa2','orange':'#d65f00','yellow':'#a68400','green':'#218739','white':'#ffffff'}
rows=[
 ('W01','X1 centre (+12V)','S12','RED','suggested','+12V'),
 ('W02','S12','U1 IN+','RED','suggested','+12V'),
 ('W03','S12','U3 POWER+ / VM','RED','suggested','+12V'),
 ('W04','X1 sleeve (12V-)','S0','BLACK','suggested','0V'),
 ('W05','S0','U1 IN-','BLACK','suggested','0V'),
 ('W06','S0','U1 OUT-','BLACK','suggested','0V'),
 ('W07','S0','U3 POWER- / GND','BLACK','suggested','0V'),
 ('W08','S0','U2 GND','BLACK','suggested','0V'),
 ('W09','S0','E1 GND / BLACK lead','BLACK','motor harness','0V'),
 ('W10','U1 OUT+ (+3.3V)','S3','BLUE','suggested','+3.3V'),
 ('W11','S3','U2 3V3','BLUE','suggested','+3.3V'),
 ('W12','S3','E1 V+ / BLUE lead','BLUE','motor harness','+3.3V'),
 ('W13','U2 GPIO0','U3 IN1','VIOLET','suggested','IN1'),
 ('W14','U2 GPIO1','U3 IN2','ORANGE','suggested','IN2'),
 ('W15','E1 A / YELLOW lead','U2 GPIO3','YELLOW','motor harness','ENC_A'),
 ('W16','E1 B / GREEN lead','U2 GPIO4','GREEN','motor harness','ENC_B'),
 ('W17','U3 OUT1','M1 RED lead','RED','motor harness','MOTOR_A'),
 ('W18','U3 OUT2','M1 WHITE lead','WHITE','motor harness','MOTOR_B'),
]
with (ROOT/'docs/WIRE_LIST.csv').open('w',newline='',encoding='utf-8') as f:
 w=csv.writer(f);w.writerow(['Wire','From terminal','To terminal','Colour','Colour basis','Net']);w.writerows(rows)

class Draw:
 def __init__(self,name,title,sheet):
  self.name=name;self.svg=[f'<svg xmlns="http://www.w3.org/2000/svg" width="{W}" height="{H}" viewBox="0 0 {W} {H}"><rect width="100%" height="100%" fill="white"/>']
  c.saveState();c.scale(page[0]/W,page[1]/H)
  self.rect(24,24,W-48,H-48,'white','#263238',1.5)
  self.text(50,68,title,29,True)
  self.text(50,99,'RB-EL-001 / REV B / 14 OCT 2026 / Firmware 2.0.2 / millimetres not applicable - schematic',16)
  self.line([(40,116),(1640,116)],'#263238',1)
  self.rect(40,1100,1600,64,'#f3f6f9','#263238',1)
  self.text(55,1126,'ESP32-C3 + DRV8871 + JGY-370 encoder motor | Terminal labels take precedence over board position.',16,True)
  self.text(55,1150,'RUN: 12V supply only. SERVICE USB: 12V OFF. Never feed ESP 3V3 from Mini360 while USB is connected.',15)
  self.text(1618,1127,f'{sheet} / 3',18,True,anchor='end')
 def rect(self,x,y,w,h,fill='white',stroke='#526274',sw=2):
  self.svg.append(f'<rect x="{x}" y="{y}" width="{w}" height="{h}" fill="{fill}" stroke="{stroke}" stroke-width="{sw}"/>')
  c.setFillColor(toColor(fill));c.setStrokeColor(toColor(stroke));c.setLineWidth(sw);c.rect(x,H-y-h,w,h,fill=1,stroke=1)
 def line(self,points,col='#263238',width=3,halo=False):
  if halo:self.line(points,'#ffffff',width+8)
  self.svg.append('<polyline points="'+' '.join(f'{x},{y}' for x,y in points)+f'" fill="none" stroke="{col}" stroke-width="{width}" stroke-linejoin="round"/>')
  p=c.beginPath();p.moveTo(points[0][0],H-points[0][1])
  for x,y in points[1:]:p.lineTo(x,H-y)
  c.setStrokeColor(toColor(col));c.setLineWidth(width);c.drawPath(p)
 def text(self,x,y,t,size=18,bold=False,col='#182b40',anchor='start'):
  self.svg.append(f'<text x="{x}" y="{y}" font-family="Arial, sans-serif" font-size="{size}" font-weight="'+('bold' if bold else 'normal')+f'" fill="{col}" text-anchor="{anchor}">{escape(t)}</text>')
  c.setFillColor(toColor(col));c.setFont('Helvetica-Bold' if bold else 'Helvetica',size)
  if anchor=='end':c.drawRightString(x,H-y,t)
  elif anchor=='middle':c.drawCentredString(x,H-y,t)
  else:c.drawString(x,H-y,t)
 def dot(self,x,y,col='#263238',r=5,fill=None):
  fill=fill or col
  self.svg.append(f'<circle cx="{x}" cy="{y}" r="{r}" stroke="{col}" fill="{fill}" stroke-width="2"/>')
  c.setFillColor(toColor(fill));c.setStrokeColor(toColor(col));c.setLineWidth(2);c.circle(x,H-y,r,fill=1,stroke=1)
 def wire(self,points,key):
  if key=='white':self.line(points,'#607080',8)
  self.line(points,C[key],3.5,halo=key!='white')
 def terminal(self,x,y,label,side='right',col='#263238'):
  self.dot(x,y,col,6,'white')
  dx=-15 if side=='right' else 15
  self.text(x+dx,y-10,label,18,True,anchor='end' if side=='right' else 'start')
 def box(self,x,y,w,h,tag,title,subtitle=''):
  self.rect(x,y,w,h)
  self.rect(x,y,w,76,'#eef3f8','#526274',1)
  self.text(x+16,y+29,tag+'  '+title,21,True)
  if subtitle:self.text(x+16,y+56,subtitle,15)
 def note(self,x,y,lines):
  for i,t in enumerate(lines):self.text(x,y+i*25,t,17)
 def finish(self):
  (OUT/(self.name+'.svg')).write_text(''.join(self.svg)+'</svg>',encoding='utf-8')
  c.restoreState();c.showPage()

d=Draw('wiring_power','DETAILED WIRING - 12V / 3.3V POWER DISTRIBUTION',1)
d.box(70,170,310,340,'X1','DC socket','5.5 x 2.1mm; check lug identity')
d.box(670,200,360,350,'U1','Mini360','Adjust OUT+ / OUT- to 3.30V')
d.box(1280,170,310,330,'U3','DRV8871','Power terminals only; control on sheet 2')
d.box(700,770,360,170,'U2','ESP32-C3','3V3 / GND power pins')
d.box(1250,770,340,170,'E1','Integral encoder','Motor harness: BLUE / BLACK')
# Routes. Splice nodes are soldered joins or equivalent same-net connections.
d.wire([(380,290),(500,290),(500,150),(1180,150),(1180,290),(1280,290)],'red')
d.wire([(500,290),(500,340),(670,340)],'red')
d.wire([(380,370),(550,370),(550,650),(1510,650)],'black')
d.wire([(550,420),(670,420)],'black')
d.wire([(1030,420),(1100,420),(1100,650)],'black')
d.wire([(1280,370),(1200,370),(1200,650)],'black')
d.wire([(1030,340),(1120,340),(1120,700),(780,700)],'blue')
d.wire([(1120,700),(1450,700)],'blue')
d.wire([(800,700),(800,770)],'blue')
d.wire([(1320,700),(1320,770)],'blue')
d.wire([(1000,650),(1000,770)],'black')
d.wire([(1490,650),(1490,770)],'black')
for x,y,col in [(500,290,'red'),(550,420,'black'),(550,650,'black'),(1100,650,'black'),(1200,650,'black'),(1000,650,'black'),(1490,650,'black'),(1120,700,'blue'),(800,700,'blue'),(1320,700,'blue')]:d.dot(x,y,C[col])
for x,y,l,s in [(380,290,'Centre contact: 12V+','right'),(380,370,'Sleeve contact: 12V-','right'),(670,340,'IN+ (12V+)','left'),(670,420,'IN- (12V-)','left'),(1030,340,'OUT+ (+3.3V)','right'),(1030,420,'OUT- (GND)','right'),(1280,290,'POWER+ / VM (12V+)','left'),(1280,370,'POWER- / GND (12V-)','left')]:d.terminal(x,y,l,s)
for x,y,l in [(800,770,'3V3'),(1000,770,'GND'),(1320,770,'V+ / BLUE'),(1490,770,'GND / BLACK')]:
 d.dot(x,y,'#263238',6,'white');d.text(x,y+100,l,17,True,anchor='middle')
d.text(390,270,'W01 RED',16);d.text(550,320,'W02 RED',16);d.text(960,140,'W03 RED',16)
d.text(390,405,'W04 BLACK',16);d.text(556,450,'W05 BLACK',16);d.text(1045,470,'W06 BLACK',16)
d.text(1212,450,'W07 BLACK',16);d.text(890,748,'W08 BLACK',16);d.text(1460,748,'W09 BLACK',16)
d.text(1135,550,'W10 BLUE',16);d.text(700,740,'W11 BLUE',16);d.text(1260,740,'W12 BLUE',16)
d.text(508,240,'S12: +12V junction',16,True)
d.text(650,625,'S0: 0V / 12V- / GND - ALL RETURNS COMMON',18,True)
d.text(1135,682,'S3: +3.3V junction',17,True,col=C['blue'])
d.note(80,970,['Solid dots = connected junctions. Crossing lines without a dot are NOT connected.',
 'S12 / S0 / S3 are drawing labels for electrical joins, not extra terminal blocks in the BOM.',
 'RED / BLACK / BLUE hookup-wire colours are suggested; E1 BLUE and BLACK are the confirmed harness colours.',
 'X1 switched third contact is unused; identify centre/sleeve with a meter, not by lug position.'])
d.finish()

d=Draw('wiring_signals','DETAILED WIRING - MOTOR DRIVE / ENCODER FEEDBACK',2)
d.box(90,240,450,550,'U2','ESP32-C3 SuperMini','Firmware pins; not physical header order')
d.box(930,200,450,300,'U3','DRV8871','IN1 / IN2 controls; OUT1 / OUT2 motor')
d.box(930,500,340,290,'E1','Integral encoder','Inside six-wire JGY-370 motor')
d.box(1270,790,320,210,'M1','12V DC motor','RED / WHITE motor leads')
d.wire([(540,340),(930,340)],'violet')
d.wire([(540,430),(930,430)],'orange')
d.wire([(540,600),(930,600)],'yellow')
d.wire([(540,690),(930,690)],'green')
d.wire([(1380,300),(1570,300),(1570,720),(1360,720),(1360,790)],'red')
d.wire([(1380,400),(1510,400),(1510,750),(1480,750),(1480,790)],'white')
for x,y,l,s in [(540,340,'GPIO0','right'),(540,430,'GPIO1','right'),(540,600,'GPIO3 (ENC_A)','right'),(540,690,'GPIO4 (ENC_B)','right'),(930,340,'IN1','left'),(930,430,'IN2','left'),(1380,300,'OUT1','right'),(1380,400,'OUT2','right'),(930,600,'A / YELLOW lead','left'),(930,690,'B / GREEN lead','left')]:d.terminal(x,y,l,s)
for x,y,l in [(1360,790,'RED'),(1480,790,'WHITE')]:d.dot(x,y,'#263238',6,'white');d.text(x,y+107,l,19,True,anchor='middle')
d.text(585,323,'W13 VIOLET (suggested)',18,True,col=C['violet'])
d.text(585,413,'W14 ORANGE (suggested)',18,True,col=C['orange'])
d.text(585,583,'W15 YELLOW (harness)',18,True,col=C['yellow'])
d.text(585,673,'W16 GREEN (harness)',18,True,col=C['green'])
d.text(1420,280,'W17 RED',18,True,col=C['red'])
d.text(1400,385,'W18 WHITE',18,True)
d.note(110,830,['U2 power: 3V3 / GND on sheet 1.', 'E1 power: BLUE -> +3.3V; BLACK -> GND (sheet 1).', 'GPIO3 and GPIO4 use firmware INPUT_PULLUP at 3.3V.', 'No separate encoder/controller board is required.'])
d.note(90,1020,['OUT1 / OUT2 reverse polarity with direction: WHITE is NOT a permanent 12V- / GND lead.',
 'Encoder BLACK = common 0V. Never apply +12V to the encoder supply or signal leads.'])
d.finish()

d=Draw('wiring_overview','HIGH-LEVEL OVERVIEW - LABELLED TERMINALS AND COLOURS',3)
d.box(60,220,280,220,'X1','12V DC inlet','External adapter; USB off')
d.box(650,170,380,230,'U1','Mini360','12V input -> regulated 3.3V')
d.box(650,470,380,340,'U3','DRV8871','Motor power + direction control')
d.box(1260,120,350,470,'U2','ESP32-C3','Web / schedules / encoder limits')
d.box(1260,630,350,415,'M1/E1','Motor + encoder','Six-wire motor harness')
# Same electrical connections as sheets 1/2, with wire IDs omitted for clarity.
d.wire([(340,300),(450,300),(450,270),(650,270)],'red')
d.wire([(450,300),(450,590),(650,590)],'red')
d.wire([(340,370),(500,370),(500,340),(650,340)],'black')
d.wire([(500,370),(500,660),(650,660)],'black')
d.wire([(1030,270),(1160,270),(1260,270)],'blue')
d.wire([(1030,340),(1190,340),(1260,340)],'black')
d.wire([(1160,270),(1160,830),(1260,830)],'blue')
d.wire([(1190,340),(1190,890),(1260,890)],'black')
d.wire([(1260,410),(1110,410),(1110,720),(1030,720)],'violet')
d.wire([(1260,470),(1080,470),(1080,770),(1030,770)],'orange')
d.wire([(1030,590),(1230,590),(1230,750),(1260,750)],'red')
d.wire([(1030,660),(1210,660),(1210,790),(1260,790)],'white')
d.wire([(1260,950),(1055,950),(1055,530),(1260,530)],'yellow')
d.wire([(1260,1010),(1040,1010),(1040,570),(1260,570)],'green')
d.wire([(650,340),(1030,340)],'black')
d.text(840,380,'IN- / OUT-: common 0V',16,anchor='middle')
d.dot(450,300,C['red']);d.dot(500,370,C['black']);d.dot(1160,270,C['blue']);d.dot(1190,340,C['black'])
for x,y,l,s in [(340,300,'12V+ (RED)','right'),(340,370,'12V- (BLACK)','right'),(650,270,'IN+ / 12V+','left'),(650,340,'IN- / 12V-','left'),(1030,270,'OUT+ / 3.3V','right'),(1030,340,'OUT- / GND','right'),(650,590,'POWER+ / VM / 12V+','left'),(650,660,'POWER- / GND / 12V-','left'),(1030,590,'OUT1 (RED)','right'),(1030,660,'OUT2 (WHITE)','right'),(1030,720,'IN1 (VIOLET)','right'),(1030,770,'IN2 (ORANGE)','right'),(1260,270,'3V3 (BLUE)','left'),(1260,340,'GND (BLACK)','left'),(1260,410,'GPIO0 -> IN1','left'),(1260,470,'GPIO1 -> IN2','left'),(1260,530,'GPIO3 <- YELLOW','left'),(1260,570,'GPIO4 <- GREEN','left'),(1260,750,'Motor RED <- OUT1','left'),(1260,790,'Motor WHITE <- OUT2','left'),(1260,830,'Encoder BLUE: +3.3V','left'),(1260,890,'Encoder BLACK: GND','left'),(1260,950,'Encoder YELLOW: A','left'),(1260,1010,'Encoder GREEN: B','left')]:d.terminal(x,y,l,s)
d.note(60,860,['GND / 0V / 12V- are the same return net.', 'Mini360 OUT- and driver POWER- join this net.', 'Dots = junctions; crossings without dots = separate nets.', '', 'Additional hookup colours are recommendations.', 'Motor / encoder harness colours are confirmed.', '', 'Detailed power: sheet 1.', 'Detailed control and feedback: sheet 2.'])
d.finish();c.save()
# Preserve the previous image URL while replacing its simplified content.
(OUT/'wiring.svg').write_bytes((OUT/'wiring_overview.svg').read_bytes())
# PNG images come from the actual vector PDF, keeping previews consistent.
try:
 import pypdfium2 as pdfium
except ImportError:
 print('SVG/PDF saved. Install pypdfium2 to generate PNG previews.')
else:
 pdf=pdfium.PdfDocument(str(PDF))
 for i,name in enumerate(['wiring_power','wiring_signals','wiring_overview']):
  pdf[i].render(scale=2).to_pil().save(OUT/(name+'.png'))
 print('Saved three SVGs, three PNGs, a three-page A3 PDF and an 18-wire terminal list.')

