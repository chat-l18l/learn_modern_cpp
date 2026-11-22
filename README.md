# C++ opnieuw leren

Dit project is een werkplaats om modern C++ te leren. Het uitgangspunt is C++-ervaring van
twintig jaar geleden. Het doel is niet om alle nieuwe taalregels te kennen. Het doel is om de
constructies te **herkennen** die nog steeds onveilig zijn, en om te weten welk gereedschap ze
vindt.

## Doel

1. Herken in code de constructies die ongedefinieerd gedrag geven of die een test niet ziet.
2. Weet per constructie welk vangnet de fout vindt: de compiler, statische analyse, een
   sanitizer, of niets.
3. Ken per constructie het veilige alternatief in C++23.
4. Leg elke stap vast, zodat de kennis terug te lezen is.

Een experiment is klaar als het antwoord op vraag 1, 2 en 3 in het verslag staat, met gemeten
uitkomsten. Een verwachting is geen uitkomst.

## Werkwijze

We werken stap voor stap. Eén stap is één experiment of één wijziging aan de werkplaats.

1. Kies een constructie uit de [catalogus](docs/CATALOGUS.md). Zet de status op "bezig".
2. Maak een experiment: `pixi run new-experiment e002 korte-naam`.
3. Schrijf in het verslag eerst de vraag en de verwachting.
4. Schrijf de onveilige code, de veilige code en de tests.
5. Draai `pixi run check`, `pixi run tidy` en `pixi run cppcheck`. Noteer wat elk vangnet meldt.
6. Schrijf het resultaat en de conclusie in het verslag.
7. Werk de catalogus, het [logboek](docs/LOGBOEK.md) en zo nodig de [besluiten](docs/BESLUITEN.md)
   bij.
8. Maak één commit voor de stap.

| Wat | Waar |
| --- | --- |
| Wat we gaan doen, in fasen | [docs/PLAN.md](docs/PLAN.md) |
| Lijst van onveilige constructies, met status | [docs/CATALOGUS.md](docs/CATALOGUS.md) |
| Genomen besluiten en voorstellen | [docs/BESLUITEN.md](docs/BESLUITEN.md) |
| Wat er per werksessie gebeurde | [docs/LOGBOEK.md](docs/LOGBOEK.md) |
| Afspraken voor AI-agents | [AGENTS.md](AGENTS.md) |

### Regels voor onveilige code

- Onveilige code staat met opzet in een bestand met `unsafe` in de naam.
- Bovenaan dat bestand staat: `THIS PROGRAM HAS UNDEFINED BEHAVIOUR ON PURPOSE`.
- Repareer onveilige code niet. De onveilige code is het onderwerp van de studie.
- De veilige vorm staat ernaast, met unit tests.

## Aan de slag

De bouwomgeving komt uit [pixi](https://pixi.sh). Pixi installeert de compilers en het
gereedschap in `.pixi/`, met de versies uit `pixi.lock`. Er is geen `sudo` nodig.

Installeer de omgeving:

```bash
pixi install
```

Bouw en test met GCC, zonder sanitizer:

```bash
pixi run test
```

Bouw en test met een andere preset:

```bash
pixi run test clang-asan
```

Doe de volledige controle, met twee compilers, met en zonder sanitizer:

```bash
pixi run check
```

### Presets

| Preset | Compiler | Optimalisatie | Vangnet |
| --- | --- | --- | --- |
| `gcc-debug` | GCC 15 | `-O0` | geen |
| `gcc-release` | GCC 15 | `-O2` | geen |
| `gcc-asan` | GCC 15 | `-O0` | AddressSanitizer en UndefinedBehaviorSanitizer |
| `clang-debug` | Clang 23 | `-O0` | geen |
| `clang-asan` | Clang 23 | `-O0` | AddressSanitizer en UndefinedBehaviorSanitizer |
| `clang-tsan` | Clang 23 | `-O0` | ThreadSanitizer |
| `clang-audit` | Clang 23 | `-O0` | Code van anderen als eigen code, plus `-Wunsafe-buffer-usage`. Nooit `-Werror`. |

Elke preset bouwt in `build/<preset>/`. De veilige code bouwt met `-Werror`. In de presets
zonder sanitizer heeft de veilige code ook een stackbudget van 512 bytes per functie
(`-Wframe-larger-than`). Tests vallen buiten dat budget. De demo's met
ongedefinieerd gedrag bouwen zonder `-Werror`: hun waarschuwingen zijn meetgegevens.

### Taken

| Taak | Wat de taak doet |
| --- | --- |
| `pixi run configure [preset]` | Maakt de buildmap voor de preset |
| `pixi run build [preset]` | Bouwt alle experimenten |
| `pixi run test [preset]` | Bouwt en draait alle tests |
| `pixi run check` | Draait `test` voor `gcc-debug`, `gcc-asan`, `clang-debug` en `clang-asan` |
| `pixi run tidy` | Draait clang-tidy over `experimenten/` |
| `pixi run cppcheck` | Draait cppcheck over de veilige code. Faalt bij een melding. |
| `pixi run cppcheck-unsafe` | Draait cppcheck alleen over de demo's. Toont de meldingen en faalt nooit. |
| `pixi run audit` | Bouwt `clang-audit` opnieuw en draait clang-tidy en cppcheck over `vreemd/`. Toont alle meldingen en faalt nooit. |
| `pixi run format` | Zet de code in de afgesproken opmaak |
| `pixi run format-check` | Controleert de opmaak, zonder te wijzigen |
| `pixi run new-experiment <id> <naam>` | Maakt een nieuw experiment uit het sjabloon |
| `pixi run clean` | Verwijdert `build/` |

De standaardpreset is `gcc-debug`.

## Opbouw van de werkmap

| Pad | Inhoud |
| --- | --- |
| `pixi.toml`, `pixi.lock` | Gereedschap, versies en taken |
| `CMakeLists.txt` | Bouwinstellingen voor het hele project |
| `CMakePresets.json` | De presets uit de tabel hierboven |
| `cmake/leren.cmake` | Waarschuwingen en sanitizer-vlaggen voor elk doel |
| `experimenten/` | Eén map per experiment |
| `experimenten/_sjabloon/` | Het sjabloon voor een nieuw experiment |
| `vreemd/` | Code van anderen, met geplante fouten. Zie [vreemd/README.md](vreemd/README.md). |
| `docs/` | Plan, catalogus, besluiten en logboek |
| `tools/` | Hulpscripts |
| `.clang-tidy`, `.clang-format`, `.clangd` | Instellingen voor statische analyse, opmaak en editor |

## Vaste termen

| Term | Betekenis |
| --- | --- |
| Ongedefinieerd gedrag (UB) | Code waarvoor de C++-standaard geen uitkomst voorschrijft. Alles kan gebeuren, ook "het werkt". |
| Vangnet | Een hulpmiddel dat een fout kan vinden: compilerwaarschuwing, statische analyse, sanitizer of test |
| Sanitizer | Extra controles die de compiler in het programma bouwt. De controles draaien tijdens het uitvoeren. |
| Debug-modus | Een versie van libstdc++ met controles op iterators. Aan met `-D_GLIBCXX_DEBUG`. Verandert de opbouw van de containers. |
| Vals alarm | Een melding van een vangnet bij code die correct is |
| Code van anderen | Code die het project overneemt en niet zelf schrijft: een bibliotheek, een pakket, gekopieerde code |
| Audit | Een controle van code van anderen met alle waarschuwingen en analyses, voordat het project de code gebruikt |
| Blinde vlek | Een fout die een vangnet aantoonbaar niet ziet |
| Wrapper | Eigen code die de enige toegang is tot code van anderen, en die alle voorwaarden controleert |
| Stackbudget | Het maximale stackframe van één functie. Hier 512 bytes, voor een stack van 4 KiB per taak. |
| Pure functie | Een functie waarvan de uitkomst alleen van de parameters afhangt, en die alleen schrijft in de buffers die ze meekrijgt |
| Closure-object | Het object dat een lambda-expressie maakt. Elke capture is een lid van dat object. |
| ASan | AddressSanitizer. Vindt geheugen dat na vrijgeven of buiten de grenzen wordt gebruikt. |
| UBSan | UndefinedBehaviorSanitizer. Vindt onder meer overflow van signed integers en foute casts. |
| TSan | ThreadSanitizer. Vindt data races tussen threads. |
| Statische analyse | Een programma dat de broncode leest zonder de code uit te voeren: clang-tidy, cppcheck |
| Hangende view | Een `std::string_view` of `std::span` die naar geheugen wijst dat niet meer bestaat |
| Eigenaar | Het object dat geheugen vrijgeeft als het object verdwijnt, bijvoorbeeld `std::string` |
| Vangnet-test | Een test die slaagt als een sanitizer de onveilige demo vangt |

## Experimenten

| ID | Onderwerp | Catalogus | Wie vindt de fout | Verslag |
| --- | --- | --- | --- | --- |
| e001 | Hangende `std::string_view` naar een tijdelijke `std::string` | G01 | ASan altijd; Clang alleen met `[[clang::lifetimebound]]`; GCC, clang-tidy en cppcheck niet | [verslag](experimenten/e001-dangling-string-view/README.md) |
| e002 | Referentie, iterator of range-for in een `std::vector` na `push_back` | G02 | ASan, cppcheck en Clang 23 altijd (de laatste twee ook vals bij `reserve`); debug-modus alleen iterators; GCC en clang-tidy niet | [verslag](experimenten/e002-vector-push-back/README.md) |
| e003 | Code van anderen: verborgen waarschuwingen en blinde vlekken | G24, G25, G26 | Gewone bouw niets (`-isystem`); `clang-audit` vindt 2 van 4 fouten precies; ASan alleen in code die uit de bron is gebouwd | [verslag](experimenten/e003-code-van-anderen/README.md) |
| e004 | Lambda met `[&]` of `[=]` die langer leeft dan wat ze vangt; waar captures staan en wat ze aan stack kosten | G03 | ASan, cppcheck, clang-tidy en Clang 23 altijd; GCC alleen `[=]` met `this` | [verslag](experimenten/e004-lambda-capture/README.md) |

## Tests

Er zijn twee soorten tests. CTest draait beide.

| Soort | Wat de test bewijst | Wanneer de test draait |
| --- | --- | --- |
| Unit test (GoogleTest) | De veilige vorm doet wat het verslag zegt | Bij elke preset |
| Vangnet-test | Een sanitizer of de debug-modus van libstdc++ vangt de onveilige demo. De test faalt als de fout ongemerkt doorgaat. | Alleen bij de preset met dat vangnet |
| Controletest | Een correcte variant van de demo slaagt, ook onder een sanitizer. Zo blijkt dat het vangnet geen vals alarm geeft. | Bij elke preset |
| Blinde-vlek-test | Een vangnet ziet een fout aantoonbaar niet. De test faalt als het vangnet de fout later wel ziet; dan moet het verslag worden bijgewerkt. | Alleen bij de preset met dat vangnet |

Een unit test die slaagt, bewijst niet dat de code veilig is. Experiment e001 laat dat zien: de
onveilige demo met een korte string drukt zonder sanitizer het juiste woord af en eindigt met
exitcode 0.
