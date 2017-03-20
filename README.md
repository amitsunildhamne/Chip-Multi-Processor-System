# Chip-Multi-Processor-System
Designed a Chip Multi Processor Memory System. Implemented a
multi-level cache and Simple DRAM and expanded the system to
incorporate multi processor system. Made the system more efficient by
incorporating Static Way Partitioning.
Performance measure done using SPEC traces.
How to run?
1. cd name-of-folder
2. cd src
3. make
4. ./sim	–h	(to	see	the	simulator	options)
5. ./sim	mode	1	../traces/*.mtr.gz			(to	test	the	default	configuration)
6.	 ../scripts/runall.sh	runs	all	configurations	for	all	traces
