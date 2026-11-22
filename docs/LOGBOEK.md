# Logboek

Eén blok per werksessie, nieuwste bovenaan. Elk blok zegt wat er gedaan is, wat er gemeten is en
wat de volgende stap is.

---

## 2026-11-22 — Stap 10: experiment e005, sterke types (G28, G29)

**Gedaan:**

1. Experiment e005: sterke handles, grootheden, een vaste tabel en een pool met generatie.
2. 24 `static_assert`-tests tijdens het compileren, vier compile-fail-tests, unit tests, en drie
   blinde-vlek-tests voor fouten zonder ongedefinieerd gedrag.
3. `leren_compile_fail_test` in `cmake/leren.cmake` (B21).
4. P2 regel 11: een sterk type ziet geen verwisseling van twee waarden van hetzelfde type.

**Gemeten:**

- `pixi run check` slaagt: 57 tests zonder sanitizer, 62 met ASan, per compiler.
- Een verouderde index en een verwisseling geven foute uitvoer. Geen sanitizer meldt iets.
- Een generatie van 16 bits loopt na 65536 keer hergebruik rond. De oude handle is dan weer geldig.
- Designated initializers in de verkeerde volgorde: fout in GCC, waarschuwing in Clang.
- clang-tidy `readability-suspicious-call-argument` en cppcheck `knownConditionTrueFalse` vonden
  de verwisseling in de demo. `bugprone-easily-swappable-parameters` meldde de constructor van
  `Sensor_handle` in onze eigen veilige code.

**Gevonden tijdens het werk:**

- De taak `cppcheck` sloeg alleen `demo_unsafe.cpp` over. Nu alle `demo_*.cpp`.
- `misc-non-private-member-variables-in-classes` botst met P1; uitgezet.
- Bewezen dat de nieuwe tests kunnen falen: zonder `explicit` stoppen vier `static_assert`s.

**Open:**

- B10 (opmaak) en B11 (commitberichten) wachten op een besluit.

**Volgende stap:** G04, range-for over een tijdelijk object in C++20 en C++23.

---

## 2026-11-22 — Stap 9: handles en grootheden als sterke types (B20)

**Gedaan:**

1. P2 regel 6 aangescherpt: een handle is een uitgeschreven struct per tabel, met een
   `explicit` constructor en alleen `==`.
2. P2 regel 10 toegevoegd: een fysieke grootheid aan een interface tussen modules is een sterk
   type met de eenheid in de naam. Tijd via `std::chrono`.
3. Besluit B20, auditvragen A21 en A22, catalogusregel G29 en een valkuil in AGENTS.md.

**Gemeten:**

- Met `static_assert` tijdens het compileren, GCC 15.3 en Clang 23.1: het sterke type weigert
  een handle van een andere tabel, een `int`, rekenen, ordenen en `sensor_at(table, {3})`.
- Een struct zonder constructor laat `sensor_at(table, {3})` door, en heeft geen `==`.

**Gevonden tijdens het werk:**

- De eerste meting gebruikte `requires`-expressies buiten een template. Die gaven harde fouten
  in plaats van `false`. Herschreven met een `concept` per eigenschap.

**Open:**

- B10 (opmaak) en B11 (commitberichten) wachten op een besluit.
- De meting van B20 staat nog niet als experiment in de repo.

**Volgende stap:** G04, range-for over een tijdelijk object in C++20 en C++23.

---

## 2026-11-22 — Stap 8: data-oriented design afgewogen (B19)

**Gedaan:**

1. De onderwerpen van data-oriented design één voor één afgewogen in een gesprek. Uitkomst:
   consensus K1–K7, vastgelegd in besluit B19.
2. P1–P4 aangevuld: transformaties in drie soorten, pure functie per element, vaste tabellen met
   een getypeerde index, `std::optional` in de struct, toestand als `enum class`-veld, één tabel
   per soort, geen framework dat de control flow overneemt.
3. Nieuw principe P7: helderheid gaat voor performance.
4. Auditvragen A17–A20 en catalogusregel G28 (verouderde index) toegevoegd.

**Status van de principes:** P1, P2, P5 en P6 vastgesteld. P3, P4 en P7 werkhypothese.

**Open:**

- B10 (opmaak) en B11 (commitberichten) wachten op een besluit.
- P7 vraagt een experiment met een heldere versie, een snelle versie en een vergelijkende test.

**Volgende stap:** G04, range-for over een tijdelijk object in C++20 en C++23.

---

## 2026-11-22 — Stap 7: twee doelen, principes en auditvragen (B18)

**Gedaan:**

1. Het doel in `README.md` herschreven naar twee doelen: leren hoe C++ misgaat, en
   ontwerpprincipes vastleggen als regels die je kunt toetsen.
2. `docs/PRINCIPES.md` gemaakt met P1–P6. Elk principe heeft een regel, een reden, een toets en
   bewijs.
3. `docs/AUDITVRAGEN.md` gemaakt met A1–A16, per principe.
4. De regeltabellen uit `AGENTS.md` naar `docs/PRINCIPES.md` verplaatst. `AGENTS.md` heeft nu
   een samenvatting en een verwijzing.
5. Besluit B18 en fase 6 (principes toetsen aan code uit de praktijk) toegevoegd.

**Status van de principes:** P1, P2, P5 en P6 vastgesteld. P3 en P4 werkhypothese: P3 heeft
alleen een voorlopige meting (G27), P4 nog geen experiment.

**Open:**

- B10 (opmaak) en B11 (commitberichten) wachten op een besluit.

**Volgende stap:** G04, range-for over een tijdelijk object in C++20 en C++23.

---

## 2026-11-22 — Stap 6: stackbudget en "de aanroeper alloceert" (B16, B17)

**Gedaan:**

1. B16: `LEREN_FRAME_LIMIT` (512 bytes) in `cmake/leren.cmake`. De veilige code bouwt met
   `-Wframe-larger-than=512` en `-Werror`, in de presets zonder sanitizer.
2. `leren_target()` kreeg de markering `HOST` voor tests. `leren_gtest()` gebruikt die.
3. B17: de ontwerpregel "de aanroeper alloceert, rekenwerk in pure functies" in AGENTS.md.

**Gemeten:**

- De grootste functie in de bibliotheekcode gebruikt 128 bytes (`gcc-debug`).
- Een geplante lokale buffer van 600 bytes stopt de bouw.
- De testbestanden gebruiken 1120–1600 bytes in de registratie van GoogleTest.
- Onder `clang-asan` gebruikt `std::vector<int>::_M_realloc_append` 536 bytes.
- `pixi run check` slaagt, ook `gcc-release`, `clang-tsan`, cppcheck, format-check en de audit.

**Gevonden tijdens het werk:**

- De eerste meting bouwde niet opnieuw en liet daarom geen enkele overschrijding zien. Een
  meting van waarschuwingen vraagt altijd een schone bouw.
- Een test met een geplante fout terugdraaien met `git checkout` werkt niet op een bestand dat
  nog niet in Git staat. Teruggedraaid met de hand.

**Open:**

- B10 (opmaak) en B11 (commitberichten) wachten op een besluit.
- Het totale stackgebruik langs een keten van aanroepen (`-fcallgraph-info=su`) is nog niet
  gemeten.

**Volgende stap:** G04, range-for over een tijdelijk object in C++20 en C++23.

---

## 2026-11-22 — Stap 5: experiment e004, lambda-captures (G03)

**Gedaan:**

1. Experiment e004 met twee ontsnappende lambda's: `[&count]` die de functie verlaat, en `[=]`
   die `this` vangt.
2. Metingen van `sizeof` van closures, van heap-gebruik door `std::function`, en van het
   stackgebruik met `-fstack-usage` en `-Wframe-larger-than=1024`.
3. Een transparant alternatief: struct `Counter`, functie `counter_tick`, tabel `Counter_table`
   met vaste grootte en `static_assert`.
4. Een regel voor lambda's in AGENTS.md ("Eenvoud boven slimheid"), twee valkuilen, en
   voorstel B16 (stackbudget per functie).

**Gemeten:**

- `pixi run check` slaagt: 41 tests zonder sanitizer, 46 met ASan, per compiler.
- ASan, cppcheck, clang-tidy en Clang 23 vinden beide fouten. GCC 15 meldt alleen `[=]` met `this`.
- Zonder sanitizer geeft `gcc-debug` bij beide fouten het juiste antwoord.
- `[&x]` kost 8 bytes. `std::function` gebruikt de heap boven 16 bytes aan captures.
- `[buffer]` met 4 KiB: stackframe 8208 bytes; `[&buffer]`: 4128 bytes; `static` buffer: 16 bytes.

**Gevonden tijdens het werk:**

- `-Werror` (B12) stopte de bouw op een test: een `const int` met een vaste waarde hoeft niet
  gevangen te worden, en Clang meldt `-Wunused-lambda-capture`.
- Het vervangen van `operator new` botst met ASan. De heap-meting draait alleen zonder sanitizer.
- ASan vangt `stack-use-after-return` nu standaard; de optie `detect_stack_use_after_return=1`
  is niet nodig.
- cppcheck gaf vals alarm (`duplicateExpression`) op de teller die `operator new` verhoogt.

**Open:**

- B10, B11 en B16 wachten op een besluit.

**Volgende stap:** G04, range-for over een tijdelijk object in C++20 en C++23.

---

## 2026-11-22 — Stap 4: regels voor eenvoudige code (B15)

**Gedaan:**

1. Een tweede deel van de gevonden richtlijn voor agents bekeken: templates, overloads,
   opbouw, `virtual` en commentaar.
2. De regels opgenomen in `AGENTS.md`, paragraaf "Eenvoud boven slimheid". Per regel staat
   erbij welk hulpmiddel de regel bewaakt.
3. Besluit B15 en catalogusregel G27 (verrassende overload) toegevoegd.

**Gemeten:**

- De bestaande code heeft geen templates, geen `virtual` en geen overloads. De code voldoet al.
- `log_it("hello")` kiest `log_it(bool)` boven `log_it(const std::string &)`. GCC 15 meldt niets.
  Clang meldt `-Wstring-conversion`, clang-tidy meldt `readability-implicit-bool-conversion`.
- De controles `modernize-use-constraints`, `readability-function-cognitive-complexity`,
  `cppcoreguidelines-slicing` en `cppcoreguidelines-virtual-class-destructor` bestaan in
  clang-tidy 23 en staan al aan.

**Open:**

- B10 (opmaak) en B11 (commitberichten) wachten op een besluit.

**Volgende stap:** G03, of fase 5 afmaken met een fuzz-test.

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
