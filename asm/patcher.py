import lief 
import zlib 

binary = lief.parse("game-of-life.elf")

segment = lief.ELF.Segment() 
segment.type = lief.ELF.Segment.TYPES.LOAD 


