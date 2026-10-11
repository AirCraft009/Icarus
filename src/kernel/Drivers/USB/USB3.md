## USB3

- based off: https://wiki.osdev.org/EXtensible_Host_Controller_Interface

### Important details
- https://www.beyondlogic.org/usbnutshell/usb1.shtml#Introduction 

### Finding a Device

- search PCI config
  - Dev has ClassID
  - Dev has SubClassID
  - Dev has interfaceNum
-  All xHCI controllers
  - ClassID = 0x0C
  - SubClassID =  0x03
  - Interface = 0x30
- Config Space
  - BAR0(32bit) & BAR1(32bit)
  - 64bit addr to mem-mapped region

### PCI
 - https://wiki.osdev.org/PCI#The_PCI_Bus

### Enhanced Configuration Mechanism
- This field is needed from ACPI for the eXtensible Host Controller Interface

