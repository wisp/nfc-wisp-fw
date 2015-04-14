#NFC-WISP Firmware
---
### Version
1.0


#Resource
---
###Tutorial & Discussion: 
See NFC-WISP wiki: http://nfc-wisp.wikispaces.com/

Please send your questions and suggestions to the [**discussion board**](http://nfc-wisp.wikispaces.com/) in wiki!!!	    

###Hardware Design repository:
https://github.com/wisp/nfc-wisp-hw.git

###Andriod App repository
Use cell-phone to updates E-ink display when loading “nfc-eink-img-update-demo” firmware 

https://github.com/wisp/nfc-reader-app.git		

#Introduction
---
1. The tagged version **"v1.0"** is the released stable version for **NFC-WISP 1.0** hardware and implement ISO 14443-Type B protocol, we put partially implemented ISO 15693 in the source code, but haven't fully tested it yet.
The older pre-release firmware code is taged as **"pre-release"**.
2. A programed **NFC-WISP 1.0** tag using this firmware can be read as normal NFC tag using some NFC reading app (such as NFC Research Lab) on NFC-enabled andriod smartphone. We test it on Nexus S, Galaxy S4 and Nexus S. But the nfc-eink-img-update-demo (see below) is only tested with Nexus S with our [andriod phone app]( https://github.com/wisp/nfc-reader-app.git).

#Configuration
---
1. Set your Code Composer Studio v6x workspace to nfc-wisp-fw/ccs and import the following projects:

    * **nfc-eink-img-update-demo**      
 An application which updates E-ink paper using [andriod phone app](	https://github.com/wisp/nfc-reader-app.git):
 * **nfc-eink-temp-accel-data-logger-demo**      
 An application which sample temerature and accelerometer motion state(in static or in motion)every 3s, and plots the result on the 2.7 inch E-ink in real time (motion state is indicated by a black bar on the right up corner).

2. Choose your build configuration under build list based on different **hardware** configuration.

    * **nfc-eink-img-update-demo**      
        - _BUILD_E_INK_2.0_BATT_   : Update [2 inch E-ink](http://www.pervasivedisplays.com/products/2inch) (Gen 2) with battery.
        - _BUILD_E_INK_2.0_BATT_FREE_   : Update [2 inch E-ink](http://www.pervasivedisplays.com/products/2inch) (Gen 2) without connecting battery.
        - _BUILD_E_INK_2.7_BATT_   : Update [2.7 inch E-ink](http://www.pervasivedisplays.com/products/27) (Gen 2) with battery.
        
  
    * **nfc-eink-temp-accel-data-logger-demo**      
        - _BUILD_E_INK_2.7_BATT_   : only works when use [2.7 inch E-ink](http://www.pervasivedisplays.com/products/27) (Gen 2) with battery
		
	* **hardware configuration**
		- Remove the Rz0 to disconnect battery
		- keep Rz0 on with re-chargeable li-ion battery connected on the back of hardware to use more energy(see [hardware repo](https://github.com/wisp/nfc-wisp-hw)).
		- by-default, the battery connection is enabled with Rz0, external FRAM chip is connected with , Accelerometer
		
3. Please read the **warning** when you build your project. 

4. If it doesn't work, please check it load the correct build file and  check if the debugger in Code Composer Studio is setted to offer 2.5V (when right-click-yourproject->Properties->Debug->MSP properties->Target Voltage(mV) should be 2500) and take a look of the **warning**.
