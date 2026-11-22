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

## Later

| Onderwerp | Waarom later |
| --- | --- |
| Continue integratie (CI) | Eerst de werkwijze lokaal bewijzen. Er is nog geen externe repository. |
| C++-modules en `import std` | CMake bouwt dan bij elk doel de std-module. Zie besluit B09. |
| Coroutines en hun levensduur | Vraagt eerst de basis uit fase 1 |
| MemorySanitizer | Vraagt een libstdc++ die met MSan is gebouwd |
