##Assignment

program, ktorý pošle zadaný súbor alebo skupinu súborov po sieti na iný počítač alebo skupinu počítačov; program bude pracovať v troch režimoch:
 - buď sa pošle priamo celý súbor alebo skupina súborov,
 - alebo sa najskôr skomprimuje pomocou Huffmanovho kódovania (je nutné urobiť vlastnú implementáciu), potom sa pošle po sieti a po prijatí sa dekomprimuje,
 - alebo sa najskôr skomprimuje algoritmom LZW (je nutné urobiť vlastnú implementáciu), potom sa pošle po sieti a po prijatí sa dekomprimuje;

###Extras

 - žiadne úniky pamäte (toto je nutné explicitne ukázať s využitím nástrojov, ktoré sú na to určené, ideálne Valgrind),
 - kód musí byť logicky rozčlenený do .h a .c súborov, pričom je potrebné dodržiavať princípy objektového programovania (hlavne zapuzdrenie),
 - semestrálna práca musí obsahovať aj jednoduchý Makefile, pomocou ktorého bude možné skompilovať jednotlivé časti semestrálnej práce,
 - so semestrálnou prácou odovzdávate:
   - programátorskú dokumentáciu, v ktorej popíšete použité štruktúry a ukážete, kde ste využili veci, ktoré musíte použiť,
   - používateľskú dokumentáciu, v ktorej popíšete spustenie a ovládanie aplikácie,
 - celá semestrálna práca bude zbalená do jedného .zip súboru, ktorý odovzdáte prostredníctvom aktivity na moodli; v .zip súbore budú iba:
   - .h súbory,
   - .c súbory,
   - vlastný Makefile,
   - programátorská a používateľská dokumentácia.
