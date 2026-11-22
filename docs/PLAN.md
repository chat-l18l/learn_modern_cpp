# Plan

Dit plan verdeelt het werk in fasen. Elke fase heeft een doel, taken en acceptatiecriteria. Een
fase is klaar als alle acceptatiecriteria gelden. Elke fase eindigt met een werkplaats waarin
`pixi run check` slaagt.

De constructies per fase staan in [CATALOGUS.md](CATALOGUS.md).

## Fase 0 — Werkplaats opzetten

**Status:** klaar op 2026-11-22.

Doel:

- Een reproduceerbare bouwomgeving met twee compilers, sanitizers en statische analyse.

Taken:

1. Pixi-omgeving met GCC, Clang, CMake, Ninja, GoogleTest, clang-tidy, cppcheck en gdb.
2. CMake-presets voor debug, release, ASan/UBSan en TSan.
3. README, AGENTS.md, plan, catalogus, besluiten en logboek.
4. Sjabloon en script voor een nieuw experiment.
5. Eerste experiment (e001) als voorbeeld van de werkwijze.

Acceptatie:

- `pixi install` op een schone machine installeert alles zonder `sudo`.
- `pixi run check` slaagt.
- De vangnet-tests van e001 slagen onder `gcc-asan` en `clang-asan`.
- `pixi run format-check` en `pixi run cppcheck` slagen.

## Fase 1 — Levensduur van objecten

Doel:

- Hangende views, referenties en iterators herkennen. Weten wanneer de compiler waarschuwt en
  wanneer alleen ASan helpt.

Taken:

1. Een experiment voor elke regel G01–G07 in de catalogus.
2. Een samenvatting: welke signalen in een code-review wijzen op een levensduurprobleem.
3. Onderzoek of `[[clang::lifetimebound]]` en `-Wlifetime-safety` als vaste gewoonte zinvol zijn.

Acceptatie:

- G01–G07 hebben de status "onderzocht".
- Elk verslag noemt het veilige alternatief en heeft een unit test voor dat alternatief.

## Fase 2 — Rekenen en geheugen

Doel:

- Zien hoe optimalisatie het gedrag van ongedefinieerde code verandert.

Taken:

1. Een experiment voor elke regel G08–G13.
2. Een preset met de geharde libstdc++ (`-D_GLIBCXX_ASSERTIONS`). Eerst als voorstel in
   [BESLUITEN.md](BESLUITEN.md).

Acceptatie:

- G08–G13 hebben de status "onderzocht".
- Minstens één experiment toont een verschil tussen `gcc-debug` en `gcc-release`.

## Fase 3 — Gelijktijdigheid

Doel:

- Data races en deadlocks vinden met ThreadSanitizer.

Taken:

1. Een experiment voor elke regel G14–G16, met de preset `clang-tsan`.
2. Voeg `clang-tsan` toe aan `pixi run check` zodra er een TSan-vangnet-test is.

Acceptatie:

- G14–G16 hebben de status "onderzocht".
- Een vangnet-test bewijst dat TSan de race uit G14 vindt.

## Fase 4 — Opbouw van het programma

Doel:

- Fouten herkennen die pas ontstaan als meerdere bestanden samenkomen.

Taken:

1. Een experiment voor elke regel G17–G21.

Acceptatie:

- G17–G21 hebben de status "onderzocht".

## Fase 5 — Code van anderen

**Status:** bezig. Besluit B14. Taak 1 en 2 zijn klaar op 2026-11-22 (experiment e003).

Doel:

- Fouten in code van anderen vinden voordat de code in een project komt, zoveel mogelijk
  tijdens het compileren.

Taken:

1. Preset `clang-audit`: code van anderen zonder `-isystem`, met `-Wunsafe-buffer-usage`, en
   clang-tidy over de headers van anderen.
2. Een experiment voor elke regel G24–G26.
3. Een experiment met een kleine C-bibliotheek met fouten erin, achter een RAII-wrapper, met
   unit tests, vangnet-tests en een fuzz-test.
4. Een protocol voor code van anderen in `AGENTS.md`, op basis van de metingen.

Acceptatie:

- G24–G26 hebben de status "onderzocht".
- De audit-preset meldt de geplante fouten in de bibliotheek van taak 3 tijdens het compileren,
  voor zover een hulpmiddel ze kan zien. Het verslag noemt de fouten die geen hulpmiddel ziet.
- Het protocol in `AGENTS.md` verwijst voor elke regel naar een meting.

## Fase 6 — Principes toetsen aan code uit de praktijk

**Status:** gepland. Besluit B18.

Doel:

- De principes uit [PRINCIPES.md](PRINCIPES.md) toetsen aan code die je in de praktijk tegenkomt:
  voorbeelden van GitHub, "slimme" trucs, en code met diepe abstracties.

Taken:

1. Kies een klein, typisch voorbeeld met een verborgen control flow of een gemengde data flow.
   Leg de herkomst vast.
2. Beantwoord de [auditvragen](AUDITVRAGEN.md) over het origineel.
3. Herschrijf het voorbeeld volgens P1–P5, met hetzelfde gedrag en dezelfde tests.
4. Meet het verschil: meldingen van compiler, clang-tidy, cppcheck en ASan; de auditvragen
   zonder aanwijsbaar antwoord; de grootte van de stackframes; het aantal bestanden dat een
   lezer moet openen om de data flow te volgen.
5. Zet het actormodel ernaast als alternatief: eigenaarschap per actor, control flow via
   berichten.

Acceptatie:

- P3 en P4 hebben de status "vastgesteld", of het verslag legt uit waarom niet.
- Het verslag noemt minstens één punt waarop het origineel beter is dan de herschrijving, of
  zegt expliciet dat er geen is.

## Later

| Onderwerp | Waarom later |
| --- | --- |
| Continue integratie (CI) | Eerst de werkwijze lokaal bewijzen. De externe repository staat op GitHub sinds 2026-11-22; GitHub Actions is de voor de hand liggende keuze. |
| C++-modules en `import std` | CMake bouwt dan bij elk doel de std-module. Zie besluit B09. |
| Coroutines en hun levensduur | Vraagt eerst de basis uit fase 1 |
| MemorySanitizer | Vraagt een libstdc++ die met MSan is gebouwd |
