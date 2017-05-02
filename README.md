SERVICES
DOCUMENTS
Untitled Document.md
NEW DOCUMENT
SAVE SESSION
Users finding bugs? Searching logs for errors? Find + fix broken code fast!
PREVIEW AS 
EXPORT AS 
SAVE TO 
IMPORT FROM 
DOCUMENT NAME


Untitled Document.md
WORDS: 1513
MARKDOWN Toggle Zen Mode PREVIEW


1
2
3
4
5
6
7
8
9
10
11
12
13
14
15
16
17
18
19
20
21
22
23
24
25
26
27
28
29
30
31
32
33
34
35
36
37
38
39
40
41
42
43
44
45
46
47
48
49
50
51
52
53
54
55
56
57
58
59
60
61
62
63
64
65
66
67
68
69
70
71
72
73
74
75
76
77
78
79
80
81
82
83
84
85
86
87
88
89
90
91
92
93
94
95
96
97
98
99
100
101
102
103
104
105
106
107
108
109
110
111
112
113
114
115
116
117
118
119
120
121
122
123
124
125
126
127
128
129
130
131
132
133
134
135
136
137
138
139
140
141
142
143
144
145
146
147
148
149
150
151
152
153
154
155
156
157
158
159
160
161
162
163
164
165
166
167
168
169
170
171
172
173
174
175
176
177
178
179
180
181
182
183
184
185
186
187
188
189
190
191
192
193
194
195
196
197
198
199
200
201
202
203
204
205
206
207
208
209
210
211
212
213
214
215
216
217
218
219
220
221
222
223
224
225
226
227
228
229
230
231
232
233
234
235
236
237
238
239
240
241
242
243
244
245
246
247
248
249
250
251
252
253
254
255
256
257
258
259
260
261
262
263
264
265
266
267
268
269
270
271
272
273
274
275
276
277
278
279
280
281
282
283
284
285
# Arduino-Monitor
A monitor system with multiple features using an Arduino Mega 2560
Short and quick readme until I have more time to go over this!
### Features:
 - Serial Communications (both for debugging and commands)
 - Powermode features for low power usage/standby
 - Read current and voltage every 100ms (readOnce, readAverage, readMin, readMax and reset stores)
 - Read ambient light every 100ms (readOnce, readAverage, readMin, readMax and reset stores)
 - Read sound signal every 100ms using fft (readFundamentalFrequency, readVolume, readAverageVolume, readMinVolume, 
readMaxVolume, readFrequencyWaveform and reset stores)
 - Visual Basic program to interface with arduino
 
 
## Overview
The arduino is designed to allow the automated testing of instrument cluster software.
## Hardware
![hardware](https://github.com/kingmercian/Arduino-Monitor/blob/master/Arduino_Box_Connections.png)
## Communications
Serial communication
baud rate = 9600
## Libraries
Will add later, for now just look at the code and find online...
## Functions
### Power Mode
#### Prerequisites
None
#### Command String
| Command | Sub-command |
| --- | --- |
| PM | XX |
#### Function
| Sub Command | Meaning | Comment |
| --- | --- | --- |
| Never Received | Power Off | Power Off |
| 0 | Power Off | Power Off (Low current mode) |
| 1 | On | Ready for use |
---
### Voltage Measurement Functions
#### Prerequisites
PM = 1
#### Operation
The voltage at the input pin is read every 100ms and the store values (Minimum, Maximum, Average) are updated.
The Average value is calculated by using the previous 5 scheduled read values, if less than 5 reading have been taken since 
power on or receipt of a &quot;Store Reset&quot; command then the average is calculated using only the available data. i.e. 
if only 3 records are available they are added together then devided by 3.
#### Command String
| Command | Sub-command |
| --- | --- |
| MVM | XX |
#### Function Overview
| Request | Response |
| --- | --- |
| Command | Meaning | String | Meaning |
| Never Received | Nothing | Nothing | Nothing |
| MVM ,0 | Nothing | Nothing | Nothing |
| MVM ,1 | Read Once | MVMR1,xx,yy | xxyy = voltage in 0.1v per bit |
| MVM ,2 | Read Average | MVMR2,xx,yy | xxyy = voltage in 0.1v per bit |
| MVM ,3 | Read Minimum | MVMR3,xx,yy | xxyy = voltage in 0.1v per bit |
| MVM ,4 | Read Maximum | MVMR4,xx,yy | xxyy = voltage in 0.1v per bit |
| MVM ,5 | Reset  Stores | MVMR5,xx | xx = 0 (NAK)xx = 1 (ACK) |
| MVM 6 -&gt; 255 | Nothing | Nothing | Nothing |
#### Function Descriptions
##### Read Once
Reads the current voltage (on demand) from the input and returns the value at a resolution of 0.1v per bit in a 16bit 
number (xx = MSB,yy = LSB).
##### Read Average
Reads the current stored Average value is returned at a resolution of 0.1v per bit in a 16bit number (xx = MSB,yy = LSB).
##### Read Minimum
Reads the Minimum voltage that has been seen since the &quot;Reset Stores&quot; command was received (or since Power On:- 
PM = 1), the value at a resolution of 0.1v per bit in a 16bit number (xx = MSB,yy = LSB).
##### Read Maximum
Reads the maximum voltage that has been seen since the &quot;Reset Stores&quot; command was received (or since Power On:- 
PM = 1), the value at a resolution of 0.1v per bit in a 16bit number (xx = MSB,yy = LSB).
##### Reset  Stores
Resets the stored valus for minimum and maximum to zero (Voltage), returns 1 if reset was successful, 0 if not successful.
---
### Current Measurement Functions
#### Prerequisites
PM = 1
#### Operation
The current at the input pin is read every 100ms and the store values (Minimum, Maximum, Average) are updated.
The Average value is calculated by using the previous 5 scheduled read values, if less than 5 reading have been taken since 
power on or receipt of a &quot;Store Reset&quot; command then the average is calculated using only the available data. i.e. 
if only 3 records are available they are added together then devided by 3.
#### Command String
| Command | Sub-command |
| --- | --- |
| MCM | XX |
#### Function Overview
| Request | Response |
| --- | --- |
| Command | Meaning | String | Meaning |
| Never Received | Nothing | Nothing | Nothing |
| MCM ,0 | Nothing | Nothing | Nothing |
| MCM ,1 | Read Once | MCMR1,xx,yy | xxyy = current in 0.1ma per bit |
| MCM ,2 | Read Average | MCMR2,xx,yy | xxyy = current in 0.1ma per bit |
| MCM ,3 | Read Minimum | MCMR3,xx,yy | xxyy = current in 0.1ma per bit |
| MCM ,4 | Read Maximum | MCMR4,xx,yy | xxyy = current in 0.1ma per bit |
| MCM ,5 | Reset  Stores | MCMR5,xx | xx = 0 (NAK)xx = 1 (ACK) |
| MCM 6 -&gt; 255 | Nothing | Nothing | Nothing |
#### Function Descriptions
##### Read Once
Reads the current (on demand) from the input and returns the value at a resolution of 0.1ma per bit in a 16bit number (xx = 
MSB,yy = LSB).
##### Read Average
Reads the current stored Average value is returned at a resolution of 0.1ma per bit in a 16bit number (xx = MSB,yy = LSB).
##### Read Minimum
Reads the Minimum current that has been seen since the &quot;Reset Stores&quot; command was received (or since Power On:- 
PM = 1), the value at a resolution of 0.1ma per bit in a 16bit number (xx = MSB,yy = LSB).
##### Read Maximum
Reads the maximum current that has been seen since the &quot;Reset Stores&quot; command was received (or since Power On:- 
PM = 1), the value at a resolution of 0.1ma per bit in a 16bit number (xx = MSB,yy = LSB).
##### Reset  Stores
Resets the stored valus for minimum and maximum to zero (Current), returns 1 if reset was successful, 0 if not successful.
---
### Ambient Light Measurement Functions
#### Prerequisites
PM = 1
#### Operation
The ambient Light at the input pin is read every 100ms and the store values (Minimum, Maximum, Average) are updated.
The Average value is calculated by using the previous 5 scheduled read values, if less than 5 reading have been taken since 
power on or receipt of a &quot;Store Reset&quot; command then the average is calculated using only the available data. i.e. 
if only 3 records are available they are added together then devided by 3.
#### Command String
| Command | Sub-command |
| --- | --- |
| MLM | XX |
#### Function Overview
| Request | Response |
| --- | --- |
| Command | Meaning | String | Meaning |
| Never Received | Nothing | Nothing | Nothing |
| MLM ,0 | Nothing | Nothing | Nothing |
| MLM ,1 | Read Once | MLMR1,xx,yy | xxyy = Light Level in 0.1lux per bit |
| MLM ,2 | Read Average | MLMR2,xx,yy | xxyy = Light Level in 0.1lux per bit |
| MLM ,3 | Read Minimum | MLMR3,xx,yy | xxyy = Light Level in 0.1lux per bit |
| MLM ,4 | Read Maximum | MLMR4,xx,yy | xxyy = Light Level in 0.1lux per bit |
| MLM ,5 | Reset  Stores | MLMR5,xx | xx = 0 (NAK)xx = 1 (ACK) |
| MLM 6 -&gt; 255 | Nothing | Nothing | Nothing |
#### Function Descriptions
##### Read Once
Reads the ambient Light (on demand) from the input and returns the value at a resolution of 0.1ma per bit in a 16bit number 
(xx = MSB,yy = LSB).
##### Read Average
Reads the ambient Light stored Average value is returned at a resolution of 0.1ma per bit in a 16bit number (xx = MSB,yy = 
LSB).
##### Read Minimum
Reads the Minimum ambient Light that has been seen since the &quot;Reset Stores&quot; command was received (or since Power 
On:- PM = 1), the value at a resolution of 0.1ma per bit in a 16bit number (xx = MSB,yy = LSB).
##### Read Maximum
Reads the maximum ambient Light that has been seen since the &quot;Reset Stores&quot; command was received (or since Power 
On:- PM = 1), the value at a resolution of 0.1ma per bit in a 16bit number (xx = MSB,yy = LSB).
##### Reset  Stores
Resets the stored valus for minimum and maximum to zero (ambient Light), returns 1 if reset was successful, 0 if not 
successful.
---
### Sound Measurement Functions
#### Prerequisites
PM = 1
#### Operation
The Sound Signal at the input pin is read on demand and the calculated values are returned using the response message.
Every 100ms a reading of sound volume is taken and the store values (Minimum, Maximum, Average) are updated.
The Average value is calculated by using the previous 5 scheduled read values, if less than 5 reading have been taken since 
power on or receipt of a &quot;Store Reset&quot; command then the average is calculated using only the available data. i.e. 
if only 3 records are available they are added together then devided by 3.
#### Command String
| Command | Sub-command |
| --- | --- |
| MSM | XX |
#### Function Overview
| Request | Response |
| --- | --- |
| Command | Meaning | String | Meaning |
| Never Received | Nothing | Nothing | Nothing |
| MSM ,0 | Nothing | Nothing | Nothing |
| MSM ,1 | Read Fundemental Frequency | MSMR1,xx,yy | xxyy = Frequency in 1hz per bit (xx = MSB,yy = LSB). |
| MSM ,2 | Read Volume Level | MSMR2,xx,yy | xxyy = Sound Volume in 1 ADC count per bit (xx = MSB,yy = LSB). |
| MSM ,3 | Read Average Volume Level | MSMR2,xx,yy | xxyy = Sound Volume in 1 ADC count per bit (xx = MSB,yy = LSB). |
| MSM ,4 | Read Minimum Volume Level | MSMR4,xx,yy | xxyy = Sound Volume in 1 ADC count per bit (xx = MSB,yy = LSB). |
| MSM ,5 | Read Maximum Volume Level | MSMR5,xx,yy | xxyy = Sound Volume in 1 ADC count per bit (xx = MSB,yy = LSB). |
| MSM ,6 | Read Frequency waveform | MSMR6,Data | Data = string of values giving Frequency bands and levels using a FFT 
algorithm (See detail below) |
| MSM ,7 | Reset  Stores | MLMR7,xx | xx = 0 (NAK)xx = 1 (ACK) |
| MSM 8 -&gt; 255 | Nothing | Nothing | Nothing |
#### Function Descriptions
##### Read Fundemental Frequency
Reads the fundamental frequency of the current input signal (on demand) and returns it in the response message at a 
resolution of xxyy = Frequency in 1hz per bit (xx = MSB,yy = LSB).
##### Read Volume Level
Reads the Sound Level (on demand) from the input and returns the value at a resolution of 1 ADC count per bit (xx = MSB,yy 
= LSB).
##### Read Average Volume Level
Reads the Sound Level stored Average value is returned at a resolution of xxyy = Frequency in 1hz per bit (xx = MSB,yy = 
LSB).
##### Read Minimum Volume Level
Reads the Minimum Sound Level that has been seen since the &quot;Reset Stores&quot; command was received (or since Power 
On:- PM = 1), the value at a resolution of xxyy = Frequency in 1hz per bit (xx = MSB,yy = LSB).
##### Read Maximum Volume Level
Reads the maximum Sound Level that has been seen since the &quot;Reset Stores&quot; command was received (or since Power 
On:- PM = 1), the value at a resolution of xxyy = Frequency in 1hz per bit (xx = MSB,yy = LSB).
##### Reset  Stores
Resets the stored valus for minimum and maximum to zero (Sound Level), returns 1 if reset was successful, 0 if not 
successful.
##### Read Frequency waveform
Reads the frequency and volume of the current input signal (on demand) and returns it in the response message according to 
the table below.
| Data Item | Value | Meaning | Resolution |
| --- | --- | --- | --- |
| 0 | MSMR6 | Id of the response message | N/A |
| 1 | 1 | Frequency band Number 1 | N/A |
| 2 | xx | Sound Level | Sound Volume in 1 ADC count per bit. |
| 3 | 2 | Frequency band Number 2 | N/A |
| 4 | xx | Sound Level | Sound Volume in 1 ADC count per bit. |
| …… |   |   |   |
| 256 | 128 | Frequency band Number 128 | N/A |
| 257 | xx | Sound Level | Sound Volume in 1 ADC count per bit. |
Arduino-Monitor
A monitor system with multiple features using an Arduino Mega 2560

Short and quick readme until I have more time to go over this!

Features:
Serial Communications (both for debugging and commands)
Powermode features for low power usage/standby
Read current and voltage every 100ms (readOnce, readAverage, readMin, readMax and reset stores)
Read ambient light every 100ms (readOnce, readAverage, readMin, readMax and reset stores)
Read sound signal every 100ms using fft (readFundamentalFrequency, readVolume, readAverageVolume, readMinVolume, readMaxVolume, readFrequencyWaveform and reset stores)
Visual Basic program to interface with arduino
Overview
The arduino is designed to allow the automated testing of instrument cluster software.

Hardware
hardware

Communications
Serial communication
baud rate = 9600

Libraries
Will add later, for now just look at the code and find online…

Functions
Power Mode
Prerequisites
None

Command String
Command	Sub-command
PM	XX
Function
Sub Command	Meaning	Comment
Never Received	Power Off	Power Off
0	Power Off	Power Off (Low current mode)
1	On	Ready for use
Voltage Measurement Functions
Prerequisites
PM = 1

Operation
The voltage at the input pin is read every 100ms and the store values (Minimum, Maximum, Average) are updated.

The Average value is calculated by using the previous 5 scheduled read values, if less than 5 reading have been taken since power on or receipt of a "Store Reset" command then the average is calculated using only the available data. i.e. if only 3 records are available they are added together then devided by 3.

Command String
Command	Sub-command
MVM	XX
Function Overview
Request	Response
Command	Meaning
Never Received	Nothing
MVM ,0	Nothing
MVM ,1	Read Once
MVM ,2	Read Average
MVM ,3	Read Minimum
MVM ,4	Read Maximum
MVM ,5	Reset Stores
MVM 6 -> 255	Nothing
Function Descriptions
Read Once
Reads the current voltage (on demand) from the input and returns the value at a resolution of 0.1v per bit in a 16bit number (xx = MSB,yy = LSB).

Read Average
Reads the current stored Average value is returned at a resolution of 0.1v per bit in a 16bit number (xx = MSB,yy = LSB).

Read Minimum
Reads the Minimum voltage that has been seen since the "Reset Stores" command was received (or since Power On:- PM = 1), the value at a resolution of 0.1v per bit in a 16bit number (xx = MSB,yy = LSB).

Read Maximum
Reads the maximum voltage that has been seen since the "Reset Stores" command was received (or since Power On:- PM = 1), the value at a resolution of 0.1v per bit in a 16bit number (xx = MSB,yy = LSB).

Reset Stores
Resets the stored valus for minimum and maximum to zero (Voltage), returns 1 if reset was successful, 0 if not successful.

Current Measurement Functions
Prerequisites
PM = 1

Operation
The current at the input pin is read every 100ms and the store values (Minimum, Maximum, Average) are updated.

The Average value is calculated by using the previous 5 scheduled read values, if less than 5 reading have been taken since power on or receipt of a "Store Reset" command then the average is calculated using only the available data. i.e. if only 3 records are available they are added together then devided by 3.

Command String
Command	Sub-command
MCM	XX
Function Overview
Request	Response
Command	Meaning
Never Received	Nothing
MCM ,0	Nothing
MCM ,1	Read Once
MCM ,2	Read Average
MCM ,3	Read Minimum
MCM ,4	Read Maximum
MCM ,5	Reset Stores
MCM 6 -> 255	Nothing
Function Descriptions
Read Once
Reads the current (on demand) from the input and returns the value at a resolution of 0.1ma per bit in a 16bit number (xx = MSB,yy = LSB).

Read Average
Reads the current stored Average value is returned at a resolution of 0.1ma per bit in a 16bit number (xx = MSB,yy = LSB).

Read Minimum
Reads the Minimum current that has been seen since the "Reset Stores" command was received (or since Power On:- PM = 1), the value at a resolution of 0.1ma per bit in a 16bit number (xx = MSB,yy = LSB).

Read Maximum
Reads the maximum current that has been seen since the "Reset Stores" command was received (or since Power On:- PM = 1), the value at a resolution of 0.1ma per bit in a 16bit number (xx = MSB,yy = LSB).

Reset Stores
Resets the stored valus for minimum and maximum to zero (Current), returns 1 if reset was successful, 0 if not successful.

Ambient Light Measurement Functions
Prerequisites
PM = 1

Operation
The ambient Light at the input pin is read every 100ms and the store values (Minimum, Maximum, Average) are updated.

The Average value is calculated by using the previous 5 scheduled read values, if less than 5 reading have been taken since power on or receipt of a "Store Reset" command then the average is calculated using only the available data. i.e. if only 3 records are available they are added together then devided by 3.

Command String
Command	Sub-command
MLM	XX
Function Overview
Request	Response
Command	Meaning
Never Received	Nothing
MLM ,0	Nothing
MLM ,1	Read Once
MLM ,2	Read Average
MLM ,3	Read Minimum
MLM ,4	Read Maximum
MLM ,5	Reset Stores
MLM 6 -> 255	Nothing
Function Descriptions
Read Once
Reads the ambient Light (on demand) from the input and returns the value at a resolution of 0.1ma per bit in a 16bit number (xx = MSB,yy = LSB).

Read Average
Reads the ambient Light stored Average value is returned at a resolution of 0.1ma per bit in a 16bit number (xx = MSB,yy = LSB).

Read Minimum
Reads the Minimum ambient Light that has been seen since the "Reset Stores" command was received (or since Power On:- PM = 1), the value at a resolution of 0.1ma per bit in a 16bit number (xx = MSB,yy = LSB).

Read Maximum
Reads the maximum ambient Light that has been seen since the "Reset Stores" command was received (or since Power On:- PM = 1), the value at a resolution of 0.1ma per bit in a 16bit number (xx = MSB,yy = LSB).

Reset Stores
Resets the stored valus for minimum and maximum to zero (ambient Light), returns 1 if reset was successful, 0 if not successful.

Sound Measurement Functions
Prerequisites
PM = 1

Operation
The Sound Signal at the input pin is read on demand and the calculated values are returned using the response message.

Every 100ms a reading of sound volume is taken and the store values (Minimum, Maximum, Average) are updated.

The Average value is calculated by using the previous 5 scheduled read values, if less than 5 reading have been taken since power on or receipt of a "Store Reset" command then the average is calculated using only the available data. i.e. if only 3 records are available they are added together then devided by 3.

Command String
Command	Sub-command
MSM	XX
Function Overview
Request	Response
Command	Meaning
Never Received	Nothing
MSM ,0	Nothing
MSM ,1	Read Fundemental Frequency
MSM ,2	Read Volume Level
MSM ,3	Read Average Volume Level
MSM ,4	Read Minimum Volume Level
MSM ,5	Read Maximum Volume Level
MSM ,6	Read Frequency waveform
MSM ,7	Reset Stores
MSM 8 -> 255	Nothing
Function Descriptions
Read Fundemental Frequency
Reads the fundamental frequency of the current input signal (on demand) and returns it in the response message at a resolution of xxyy = Frequency in 1hz per bit (xx = MSB,yy = LSB).

Read Volume Level
Reads the Sound Level (on demand) from the input and returns the value at a resolution of 1 ADC count per bit (xx = MSB,yy = LSB).

Read Average Volume Level
Reads the Sound Level stored Average value is returned at a resolution of xxyy = Frequency in 1hz per bit (xx = MSB,yy = LSB).

Read Minimum Volume Level
Reads the Minimum Sound Level that has been seen since the "Reset Stores" command was received (or since Power On:- PM = 1), the value at a resolution of xxyy = Frequency in 1hz per bit (xx = MSB,yy = LSB).

Read Maximum Volume Level
Reads the maximum Sound Level that has been seen since the "Reset Stores" command was received (or since Power On:- PM = 1), the value at a resolution of xxyy = Frequency in 1hz per bit (xx = MSB,yy = LSB).

Reset Stores
Resets the stored valus for minimum and maximum to zero (Sound Level), returns 1 if reset was successful, 0 if not successful.

Read Frequency waveform
Reads the frequency and volume of the current input signal (on demand) and returns it in the response message according to the table below.

Data Item	Value	Meaning	Resolution
0	MSMR6	Id of the response message	N/A
1	1	Frequency band Number 1	N/A
2	xx	Sound Level	Sound Volume in 1 ADC count per bit.
3	2	Frequency band Number 2	N/A
4	xx	Sound Level	Sound Volume in 1 ADC count per bit.
……			
256	128	Frequency band Number 128	N/A
257	xx	Sound Level	Sound Volume in 1 ADC count per bit.
