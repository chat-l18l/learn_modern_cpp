# Auditvragen

Deze vragen toetsen code en architectuur aan de [ontwerpprincipes](PRINCIPES.md). Ze werken voor
eigen code, voor code van anderen en voor een review van een wijziging. Elke vraag heeft een
antwoord dat je in de code kunt aanwijzen. "Dat weet de ontwikkelaar" is geen antwoord.

## Gebruik

1. Kies de eenheid: één taak, één module of één wijziging.
2. Beantwoord elke vraag. Schrijf bij elk antwoord het bestand en de regel.
3. Een vraag zonder aanwijsbaar antwoord is een bevinding.
4. Draai daarna de hulpmiddelen uit de kolom "Hulpmiddel". Een hulpmiddel vervangt de vraag
   niet: in e001 vond geen enkel statisch hulpmiddel de fout.

## Data en eigenaarschap (P1, P2)

| ID | Vraag | Hulpmiddel |
| --- | --- | --- |
| A1 | Waar staat elke struct met toestand: op de stack, in `static` opslag of op de heap? Wie is de eigenaar? | `-fstack-usage` |
| A2 | Wie mag elk stuk data veranderen? Is dat één plek? | `misc-const-correctness` |
| A3 | Leeft elke view, referentie, pointer, iterator en referentie-capture korter dan de eigenaar van de data? | Clang `-Wlifetime-safety`, cppcheck, ASan |
| A4 | Heeft een struct met toestand een lid dat een pointer, referentie of view is? Zo ja: naar wat, en wie garandeert de levensduur? | review |
| A5 | Welke invarianten heeft de data? Maken de types een ongeldige toestand onmogelijk, of controleert een functie de invariant? Welke functie? | review |

## Control flow (P1, P3)

| ID | Vraag | Hulpmiddel |
| --- | --- | --- |
| A6 | Welke toestanden heeft deze taak? Staan ze in één `enum class`? | review |
| A7 | Waar worden de overgangen berekend? Op één plek, met een `switch` zonder `default`? | `-Wswitch` |
| A8 | Is er control flow die je niet ziet op de plek van de aanroep: een overload, een impliciete conversie, een virtuele aanroep, een callback, een exceptie? | `-Wconversion`, `-Wstring-conversion`, `readability-implicit-bool-conversion` |
| A9 | Waar staan invoer en uitvoer (hardware, bestanden, netwerk, tijd)? Is het rekenwerk daartussen pure functies? | review |

## Abstractie (P4)

| ID | Vraag | Hulpmiddel |
| --- | --- | --- |
| A10 | Kun je voor elke template, elke `virtual` en elke callback de concrete reden aanwijzen? Wat zou er zonder die abstractie moeilijker zijn? | review, `readability-function-cognitive-complexity` |

## Geheugen (P5)

| ID | Vraag | Hulpmiddel |
| --- | --- | --- |
| A11 | Alloceert een functie zelf geheugen: `new`, een `std::string` of `std::vector` die groeit, een `std::function` met grote captures? | review, `cppcoreguidelines-owning-memory` |
| A12 | Past elke functie in het stackbudget? Wat is de diepste keten van aanroepen, en hoeveel stack gebruikt die? | `-Wframe-larger-than`, `-fstack-usage`, `-fcallgraph-info=su` |
| A13 | Heeft elke struct met toestand een `static_assert` op `sizeof`? | compiler |

## Code van anderen (P6)

| ID | Vraag | Hulpmiddel |
| --- | --- | --- |
| A14 | Welke code komt van anderen? Is die gecontroleerd met alle waarschuwingen aan, en uit de bron gebouwd in een preset met een sanitizer? | `pixi run audit`, ASan |
| A15 | Loopt elke aanroep naar code van anderen via één wrapper? Controleert de wrapper elke voorwaarde die de code van anderen aan de aanroeper overlaat? | review |

## Bewijs

| ID | Vraag | Hulpmiddel |
| --- | --- | --- |
| A16 | Draaien de tests onder ASan en UBSan, met `-fno-sanitize-recover=all`? Draaien ze in debug én in release? | `pixi run check`, `pixi run test gcc-release` |
