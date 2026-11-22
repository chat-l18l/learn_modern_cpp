# Logboek

Eén blok per werksessie, nieuwste bovenaan. Elk blok zegt wat er gedaan is, wat er gemeten is en
wat de volgende stap is.

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
