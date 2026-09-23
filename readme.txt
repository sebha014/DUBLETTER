Hitta dubbletter
================

- Ungefärligt antal timmar spenderade på de två delarna av labben (valfritt):

del a:
del b: 


- Vad är tidskomplexiteten på följande operationer i din hashtabell?

insert_key: O(1)
find_key: O(1)
erase_key: O(1)
värsta möjliga för samtliga blir dock O(n) om kollisioner uppstår,
detta eftersom att flera nycklar då "hamnar" på samma plats.

- Antag att vi vill sätta in många element i en Hash_Map<int, int>. Vi sätter
  därmed kapaciteten till 1000. Sedan sätter vi in element med nycklarna
  1000, 2000, 3000, 4000, och så vidare tills vi har satt in 500 element.
  Kommer hashtabellen fungera bra i det här fallet? Varför/varför inte?

Den kommer fungera men eftersom att alla nycklar får samma hash leder det till
många kollisioner vilket i sin tid ledder till att vi måste söka igenom många platser
för att hitta rätt eftersom att linear probing används. Hashtabellen blir därav
ineffektiv.

- När testprogrammet jämför Hash_Map med std::unordered_map (alternativ 9) så
  använder vi int som både nyckel och värde. Typen int är lite speciell i
  avseendet att både hashning och jämförelse av int är mycket billiga. Titta
  på resultaten av jämförelsen, med fokus på "max chain". Vad tror du skulle
  hända med exempelvis tiden för uppslagning om vi i stället använder strängar
  som nyckel? Anta att strängarna är långa (ca 1000 tecken vardera) och att
  de första 500 tecknen i alla strängar är desamma.

  Vi antar att det hade tagit mycket längre tid att hitta eftersom hasning och
  jämförelse av strängar är "dyrare" än integer. Om de 500 första tecken är lika
  måste det även gå igenom väldigt många tecken innan man hittar en skillnad.


- Vad är tidskomplexiteten på "slow.cpp" och din implementation av "fast.cpp",
  uttryckt i antalet bilder (n).

slow: O(n^2)
Detta eftersom slow jämför bilderna i par, när antalet bilder ökar växer jämförelserna
som (n*n=n^2)
fast: O(n)
Detta eftersom fast beräknar en sammanfattning för alla bilder och lagrar sedan 
bilden i en hashtabell. Då behandlas alla bilder en gång istället för två.


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
| tiny   |   69 ms   | 116 ms |  61 ms |
| small  |  471 ms   | 703 ms | 455 ms |
| medium |  2019 ms  | 3218 ms| 1948 ms|
| large  |           |        |        | Hittar ej någon large i givenfiles
|--------+-----------+--------+--------|


- Testa olika värden på "summary_size" (exempelvis mellan 6 och 10). Hur
  påverkar detta vilka dubbletter som hittas i datamängden "large"?

Hittar ingen large så går ej att testa hur olika värden på summary_size påverkar.
Generellt gäller likt dem andra dock att ett högre värde ger en bättre / mer detaljerad
sammanfattning.

- Algoritmen som implementeras i "compute_summary" kan ses som att vi beräknar
  en hash av en bild. Det är dock inte helt lätt att hitta en bra sådan funktion
  som helt motsvarar vad vi egentligen är ute efter. Vilken eller vilka
  egenskaper behöver "compute_summary" ha för att vi ska kunna använda den för
  att hitta bilder som liknar varandra? (Dvs. vilka egenskaper förväntar sig
  kod som *använder* "compute_summary"?) Tycker du att implementationen av
  "compute_summary" som är given i labbhandledningen uppfyller dessa egenskaper?

compute_summary ska ge samma resultat för samma bild och helst samma eller ett likt
resultat på om dem liknar varandra men samtidigt ksa olika bilder få olika resultat.
Samtdigt ska compute_summary vara så snabb som möjligt att beräkna / jämföra.
Vi tycker att den fungerar bra men eftersom att bilderna förenklas kan det bli fel.

- Ser du några problem med metoden för att se om två bilder är lika dana?
  Fundera exempelvis på vilka typer av olikheter som tolereras, och vilka
  typer av olikheter som anses vara för stora. Matchar detta din uppfattning
  om vad som borde vara lika?

Ja, metoden kan t.ex. säga att två bilder är likadana även om det finns mindre skillnader
i dem. Två bilder som ser lika ut kan också räknas som inte lika. Därav stämmer ibland 
inte resultatet överens men vad som ser ut att vara samma bild för oss.