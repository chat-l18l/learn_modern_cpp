# Logboek

Eén blok per werksessie, nieuwste bovenaan. Elk blok zegt wat er gedaan is, wat er gemeten is en
wat de volgende stap is.

---

## 2026-11-22 — Stap 3: B12–B14 uitgevoerd, experiment e003 (G24–G26)

**Gedaan:**

1. B12: `leren_target(<doel> [UNSAFE])`. De veilige code bouwt met `-Werror`, de demo's niet.
2. B13: `-Wcast-align` en `-Wdouble-promotion` toegevoegd.
3. B14: bibliotheek `vreemd/minilib` met vier geplante fouten, preset `clang-audit`, taken
   `audit-build` en `audit`, en de doelen `minilib` en `minilib_prebuilt`.
4. Experiment e003 met een wrapper om `minilib`, unit tests, vangnet-tests en een
   blinde-vlek-test.
5. AGENTS.md: regels voor `UNSAFE`, voor code van anderen, en drie nieuwe valkuilen.

**Gemeten:**

- `pixi run check` slaagt: 29 tests zonder sanitizer, 36 met ASan, per compiler.
- `-Werror` werkt: een geplante ongebruikte variabele stopt de bouw.
- De gewone bouw geeft 0 waarschuwingen over `minilib`. `clang-audit` vindt V1 en V3 precies,
  en wijst V2 en V4 aan als rauwe pointer-index.
- ASan ziet V4 niet in de al gebouwde bibliotheek. V2 ziet ASan wel, omdat `ml_sum` `inline` is.
- V3 is geen ongedefinieerd gedrag. Alleen de audit en `-fsanitize=implicit-conversion` zien V3.

**Gevonden tijdens het werk:**

- In de audit maakte `-Werror` de bouw kapot op de fouten van `minilib`. De audit gebruikt nu
  nooit `-Werror`.
- cppcheck negeert `-isystem` en vond `minilib` via de wrapper. De taak `cppcheck` slaat
  `vreemd/` nu over.
- clang-tidy stelde `std::in_range` (C++20) voor in plaats van een eigen grenscontrole. Overgenomen.

**Open:**

- B10 (opmaak) en B11 (commitberichten) wachten op een besluit.
- Fase 5, taak 3 en 4: fuzz-test en het protocol voor code van anderen.

**Volgende stap:** G03, of fase 5 afmaken met een fuzz-test.

---

## 2026-11-22 — Stap 2: een richtlijn voor agents bekeken

**Gedaan:**

1. Een gevonden richtlijn voor C++-agents vergeleken met onze opzet.
2. De beweringen uit de richtlijn en drie risico's van code van anderen gemeten, in een losse
   map buiten de repo. Nog geen experiment.
3. Voorstellen B12–B14, catalogusregels G22–G26 en fase 5 in het plan vastgelegd.

**Gemeten:**

- `-Wlifetime` bestaat niet in GCC 15 en Clang 23. Met GCC stopt de build.
- UBSan zonder `-fno-sanitize-recover=all`: de fout staat in de uitvoer, het programma eindigt
  met exitcode 0.
- Een header via `-isystem` geeft geen enkele waarschuwing meer (GCC 2 → 0, Clang 4 → 0).
- ASan ziet niet dat een bibliotheek zonder ASan buiten een array leest.
- Clang `-Wunsafe-buffer-usage` meldt indexeren via een rauwe pointer tijdens het compileren.

**Open:**

- B10–B14 wachten op een besluit.

**Volgende stap:** afhankelijk van het besluit: B12–B14 uitvoeren, of G03.

---

## 2026-11-22 — Stap 1: experiment e002, `push_back` en referenties (G02)

**Gedaan:**

1. Experiment e002 gemaakt op basis van het idee: `int& first = v[0]; v.push_back(4);`.
   Uitgebreid met een iterator, een range-for met `push_back`, en een correcte variant met
   `reserve(4)`.
2. De demo bouwt ook met de debug-modus van libstdc++ (`_GLIBCXX_DEBUG`).
3. Vangnet-tests voor ASan en voor de debug-modus, en een controletest voor de correcte variant.
4. `pixi run cppcheck` gesplitst: `cppcheck` controleert de veilige code en moet schoon zijn,
   `cppcheck-unsafe` meet op de demo's en faalt nooit.

**Gemeten:**

- `pixi run check` slaagt: 19 tests zonder sanitizer, 23 met ASan, per compiler.
- ASan, cppcheck en Clang 23 vinden alle drie de fouten. GCC 15 en clang-tidy niets.
- Clang 23 (`-Wlifetime-safety-invalidation`) en cppcheck melden ook de correcte code met
  `reserve()`. Clang meldt bovendien de veilige index-lus in `append_evens`.
- De debug-modus vangt de iterator en de range-for, niet de referentie.
- De range-for geeft onder `gcc-release` het juiste antwoord en onder `gcc-debug` niet.

**Gevonden tijdens het werk:**

- De debug-modus stopt met `abort()`. CTest rekent dat als fout, ook als de melding klopt.
  Opgelost met `sh -c "... || true"`. Opgenomen in AGENTS.md onder "Valkuilen".
- Een test die `v.data()` vergelijkt met een adres van vóór `push_back`, gebruikt een ongeldige
  pointerwaarde. Vervangen door een vergelijking van `capacity()`.

**Open:**

- B10 (opmaak) en B11 (commitberichten) wachten op een besluit.

**Volgende stap:** G03, een lambda die een lokale variabele by reference vangt.

---

## 2026-11-22 — Stap 0: werkplaats en experiment e001

**Gedaan:**

1. Pixi-omgeving gemaakt met conda-forge. Versies: GCC 15.3.0, Clang 23.1.6, CMake 4.5.1,
   Ninja 1.13, GoogleTest 1.18, cppcheck 2.22, clang-tidy 23.1.6, gdb 18.1.
2. Zes CMake-presets gemaakt: `gcc-debug`, `gcc-release`, `gcc-asan`, `clang-debug`,
   `clang-asan`, `clang-tsan`.
3. README, AGENTS.md, CLAUDE.md, plan, catalogus (G01–G21) en besluiten (B01–B11) geschreven.
   Werkwijze en documentstijl overgenomen uit Omnibot SDC2026 en Energy Owl.
4. Experiment e001 gedaan: hangende `std::string_view` (G01).

**Gemeten:**

- `pixi run check` slaagt: 7 unit tests per preset, plus 3 vangnet-tests onder elke ASan-preset.
- `pixi run tidy` en `pixi run cppcheck` vinden de hangende view in e001 niet. clang-tidy stelt
  wel een wijziging voor (`modernize-use-string-view`) die de fout alleen bij een letterlijke
  tekst laat verdwijnen.
- Clang 23 waarschuwt alleen als de parameter `[[clang::lifetimebound]]` heeft. GCC 15 waarschuwt
  nooit.
- Met een korte string drukt de onveilige demo zonder sanitizer het juiste woord af. Exitcode 0.
- ASan meldt de fout bij de vergelijking op regel 38. De `printf` op regel 37 leest hetzelfde
  vrijgegeven geheugen, maar ASan meldt die regel niet.

**Gevonden tijdens de opzet:**

- CMake 4.5 bouwt met C++23 de std-module voor elk doel. Uitgezet (B09).
- De ASan-meldingen van Clang hadden geen regelnummers. Oorzaak: `llvm-symbolizer` ontbrak.
  Opgelost met het pakket `llvm-tools`.
- cppcheck kende de macro `TEST()` niet. Opgelost met `--library=googletest`.

**Open:**

- B10 (opmaak) en B11 (commitberichten) wachten op een besluit.

**Volgende stap:** G02, iterator ongeldig na `push_back`. Gedaan in stap 1.
