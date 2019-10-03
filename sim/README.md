# MIPS Simulator  
A simiple (but buggy!!!orz) simulator that takes MIPS machine code as input and executes it one by one.  
## How to use?  
Run ```./sim {{binary}}```  
Here, binary is expected to be written in Big Endian (I guess so...).  
In case it is in Little Endian, you can first run ```./swap {{binary}} {{output_filename}}```  
## technical notes  
- MIPS does not have NOP instruction. So, I allocated it substituting 'addiu' (0x09), which seemed not usefull.  
## Why are you writing this in English?  
kakkotsuketadake  
