Hitta dubbletter
================

- Ungefärligt antal timmar spenderade på de två delarna av labben (valfritt):

del a:
del b:


- Vad är tidskomplexiteten på följande operationer i din hashtabell?

insert_key:
  find_key:
 erase_key:


- Antag att vi vill sätta in många element i en Hash_Map<int, int>. Vi sätter
  därmed kapaciteten till 1000. Sedan sätter vi in element med nycklarna
  1000, 2000, 3000, 4000, och så vidare tills vi har satt in 500 element.
  Kommer hashtabellen fungera bra i det här fallet? Varför/varför inte?


- När testprogrammet jämför Hash_Map med std::unordered_map (alternativ 9) så
  använder vi int som både nyckel och värde. Typen int är lite speciell i
  avseendet att både hashning och jämförelse av int är mycket billiga. Titta
  på resultaten av jämförelsen, med fokus på "max chain". Vad tror du skulle
  hända med exempelvis tiden för uppslagning om vi i stället använder strängar
  som nyckel? Anta att strängarna är långa (ca 1000 tecken vardera) och att
  de första 500 tecknen i alla strängar är desamma.


- Vad är tidskomplexiteten på "slow.cpp" och din implementation av "fast.cpp",
  uttryckt i antalet bilder (n).

slow:
fast:


- Hur lång tid tar det att köra "slow.cpp" respektive "fast.cpp" på de olika
  datamängderna?
  Tips: Använd flaggan "--nowindow" för enklare tidsmätning.
  Tips: Det är okej att uppskatta tidsåtgången för de fall du inte orkar vänta
  på att slow blir klar med det största testfallet. Om du gör detta är det en
  bra idé att beräkna tiden det tar att läsa in och skala ner bilderna separat
  från tiden det tar att jämföra bilderna. (Varför?) Du kan anta att inläsning
  är det dyra, och att inläsning tar lika lång tid i både "slow" och "fast"

|--------+-----------+--------+--------|
|        | inläsning |  slow  |  fast  |
|--------+-----------+--------+--------|
| tiny   |           |        |        |
| small  |           |        |        |
| medium |           |        |        |
| large  |           |        |        |
|--------+-----------+--------+--------|


- Testa olika värden på "summary_size" (exempelvis mellan 6 och 10). Hur
  påverkar detta vilka dubbletter som hittas i datamängden "large"?


- Algoritmen som implementeras i "compute_summary" kan ses som att vi beräknar
  en hash av en bild. Det är dock inte helt lätt att hitta en bra sådan funktion
  som helt motsvarar vad vi egentligen är ute efter. Vilken eller vilka
  egenskaper behöver "compute_summary" ha för att vi ska kunna använda den för
  att hitta bilder som liknar varandra? (Dvs. vilka egenskaper förväntar sig
  kod som *använder* "compute_summary"?) Tycker du att implementationen av
  "compute_summary" som är given i labbhandledningen uppfyller dessa egenskaper?


- Ser du några problem med metoden för att se om två bilder är lika dana?
  Fundera exempelvis på vilka typer av olikheter som tolereras, och vilka
  typer av olikheter som anses vara för stora. Matchar detta din uppfattning
  om vad som borde vara lika?

