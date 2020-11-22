EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr USLetter 11000 8500
encoding utf-8
Sheet 1 3
Title "Raspberry Pi CM4 molliebus carrier"
Date "2020-11-21"
Rev "1"
Comp "WorlickWerx"
Comment1 "CERN-OHL-1.2"
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Sheet
S 5050 2250 1600 900 
U 5FBFD3A1
F0 "cm4_highspeed" 50
F1 "cm4_highspeed.sch" 50
$EndSheet
$Sheet
S 5050 3900 1600 1300
U 5FBFD3DF
F0 "cm4_gpio" 50
F1 "cm4_gpio.sch" 50
F2 "+5V" I R 6650 4400 50 
F3 "+3V3" O R 6650 4700 50 
F4 "~EXTRST~" O L 5050 4850 50 
F5 "GLOBAL_EN" I L 5050 4600 50 
F6 "SCL0" O L 5050 4100 50 
F7 "SDA0" O L 5050 4200 50 
$EndSheet
$EndSCHEMATC
