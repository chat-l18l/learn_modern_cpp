# Agent notes — C++ opnieuw leren

Dit bestand is de bron van waarheid voor elke AI-agent in deze repo. `CLAUDE.md` verwijst
hiernaar. Lees ook `README.md`: daar staan het doel en de werkwijze.

## Wat deze repo is

Een leerproject. De gebruiker kent C++ van twintig jaar geleden en leert modern C++ (C++23).
De focus ligt op het **herkennen** van constructies die onveilig zijn of die een test niet ziet.
Elk experiment meet welk vangnet de fout vindt.

De agent is hier leermeester en lab-assistent. Leg uit waarom, niet alleen wat. Verwijs naar de
regel in de standaard of naar de documentatie van het gereedschap als dat helpt.

## Werkwijze: stap voor stap

- Werk één stap tegelijk. Eén stap is één experiment of één wijziging aan de werkplaats.
- Stel aan het begin van een stap de vraag en de verwachting op. Pas daarna code.
- Meet. Schrijf alleen uitkomsten op die je zelf hebt gezien, met de versie van het gereedschap.
  Een verwachting staat als verwachting in de tekst, niet als feit.
- Sluit elke stap af met:
  1. het verslag van het experiment (`experimenten/eNNN-*/README.md`);
  2. de status in `docs/CATALOGUS.md`;
  3. een regel in `docs/LOGBOEK.md`;
  4. een besluit in `docs/BESLUITEN.md` als er een keuze is gemaakt;
  5. de tabel "Experimenten" in `README.md`;
  6. één commit.
- Een nieuwe fase of grote wijziging begint met een plan in `docs/PLAN.md`: doel, taken en
  acceptatiecriteria per fase.
- Een open keuze die de gebruiker moet maken, komt als **Voorstel** in `docs/BESLUITEN.md`.
  Pas na akkoord wordt het een **Besluit**.
- Opgeloste of afgeronde punten blijven in de lijsten staan als historie. Zet erbij wat er
  gebeurde, met datum. Verwijder ze niet.

## Commando's

Alles draait via pixi, vanaf de root van de repo. Gebruik geen systeemcompiler.

| Doel | Commando |
| --- | --- |
| Omgeving installeren | `pixi install` |
| Bouwen en testen, één preset | `pixi run test <preset>` |
| Volledige controle | `pixi run check` |
| clang-tidy | `pixi run tidy` |
| cppcheck over de veilige code (moet schoon zijn) | `pixi run cppcheck` |
| cppcheck over de demo's (meting) | `pixi run cppcheck-unsafe` |
| Audit van code van anderen | `pixi run audit` |
| Opmaak toepassen / controleren | `pixi run format` / `pixi run format-check` |
| Nieuw experiment | `pixi run new-experiment eNNN korte-naam` |

Presets: `gcc-debug`, `gcc-release`, `gcc-asan`, `clang-debug`, `clang-asan`, `clang-tsan`,
`clang-audit`.

## Verificatie

Elke wijziging krijgt een verificatierecept met de exacte commando's. Minimaal:

```bash
pixi run check
pixi run format-check
```

Meld eerlijk wat faalt, met de uitvoer. Meld ook welke presets je niet hebt gedraaid.

## Opbouw van een experiment

```text
experimenten/eNNN-korte-naam/
  README.md            verslag: vraag, verwachting, opzet, resultaat, conclusie
  CMakeLists.txt       doelen en vangnet-tests
  <naam>.hpp/.cpp      de API die we bestuderen
  <naam>_test.cpp      unit tests van de veilige vorm (GoogleTest)
  demo_unsafe.cpp      de onveilige vorm, met opzet ongedefinieerd gedrag
```

- Voeg elk nieuw experiment toe aan `experimenten/CMakeLists.txt`.
- Elk doel roept `leren_target(<doel>)` aan. Testbinaries gebruiken `leren_gtest(...)`.
- Een demo met ongedefinieerd gedrag roept `leren_target(<doel> UNSAFE)` aan. Alle andere doelen
  bouwen met `-Werror` (besluit B12). Gebruik `UNSAFE` nooit om een waarschuwing in veilige
  code weg te krijgen.
- Een vangnet-test bewijst dat een sanitizer de demo vangt. Gebruik
  `PASS_REGULAR_EXPRESSION` met de exacte melding van de sanitizer, niet `WILL_FAIL`.
  `WILL_FAIL` slaagt ook bij een crash om een andere reden.
- Registreer geen test die afhangt van de uitkomst van ongedefinieerd gedrag zonder sanitizer.
  Die uitkomst verandert met compiler, vlaggen en geheugenindeling.
- Een blinde-vlek-test (`FAIL_REGULAR_EXPRESSION` op de melding van het vangnet) mag alleen
  als het verslag de blinde vlek heeft gemeten. Zie e003.
- Heeft een demo ook een correcte variant, registreer die dan als controletest onder elke
  preset. Die test bewijst dat het vangnet geen vals alarm geeft.
- Een vals alarm in de veilige code onderdruk je zo klein mogelijk (`#pragma clang diagnostic`
  binnen `#if defined(__clang__)`, of `// cppcheck-suppress <id>`). Zet de reden erbij en
  noteer het vals alarm in het verslag. Het vals alarm is ook een meetresultaat.

## Onveilige code

- Onveilige code staat in een bestand met `unsafe` in de naam.
- Bovenaan staat: `THIS PROGRAM HAS UNDEFINED BEHAVIOUR ON PURPOSE. Do not fix it.`
- Repareer onveilige code nooit, ook niet als clang-tidy erom vraagt. Zet een opmerking bij
  de onveilige regel die zegt wat er misgaat.
- De demo's bouwen zonder `-Werror`: hun waarschuwingen zijn meetgegevens. De veilige code
  bouwt met `-Werror`.

## Code van anderen

Code van anderen staat in `vreemd/`. Daar gelden de regels uit B14 en de metingen uit e003.

- Draai `pixi run audit` voordat code van anderen in gebruik komt, en na elke update. Lees elke
  melding. De gewone bouw meldt niets over code van anderen: CMake voegt de headers in als
  `-isystem` (e003).
- Bouw code van anderen uit de bron mee in de presets met een sanitizer. ASan ziet geen fouten
  in een bibliotheek die al gebouwd is (e003). Kan dat niet, noem de bibliotheek in het
  verslag als blinde vlek.
- Roep code van anderen alleen aan via één wrapper. De wrapper gebruikt `std::span`,
  `std::in_range` en `std::optional` of `std::expected`, en controleert elke voorwaarde die de
  code van anderen aan de aanroeper overlaat.
- Repareer de fouten in `vreemd/` niet: het zijn studieobjecten.

## Codeerafspraken

- C++23, geen compiler-extensies. Geen C++-modules (zie besluit B09).
- Code, namen en commentaar in het Engels. Documenten in het Nederlands.
- Namespace per experiment: `e001`, `e002`, ...
- Kies **tabellen en expliciete state machines** boven ketens van `if`/`else`. Gebruik een
  `enum class` met een `switch` zonder `default`, zodat `-Wswitch` een vergeten geval meldt.
- `assert()` alleen voor programmeerfouten: een gebroken invariant of een fout gebruik van de
  API. Nooit voor invoer van buiten. Invoer van buiten valideer je en je geeft een fout terug,
  bijvoorbeeld met `std::expected`.
- Commentaar beschrijft bedoeling en contract, niet de syntaxis. Zeg bij een view of
  referentie altijd wie de eigenaar is en hoe lang de eigenaar moet leven.
- Opmaak komt uit `.clang-format`. Draai `pixi run format` voor een commit.

## Schrijfstijl van documenten

- Nederlands, volgens de principes van Simplified Technical English (ASD-STE100):
  korte zinnen, één gedachte per zin, actieve vorm, tegenwoordige tijd.
- Eén begrip is één woord. Vaste termen staan in de tabel "Vaste termen" in `README.md`.
- Volgordes zijn genummerde lijsten. Opsommingen met kenmerken zijn tabellen.
- Doel en resultaat vooraan, geschiedenis achteraan.
- Een document staat op zichzelf: geen uitleg over de schrijfstijl, geen notities aan de
  gebruiker. Die horen in het gesprek of in de commit message.

## Git

- Eén commit per stap. Draai de verificatie voor de commit.
- Commitberichten in het Nederlands. Onderwerpregel: een hele zin die zegt wat er nu waar
  is, zonder punt, maximaal 72 tekens. Noem het experiment- of catalogus-ID als dat past.
  Voorbeeld: `e002: iterator ongeldig na push_back, ASan vangt het (G02)`.
- Commit of push alleen als de gebruiker erom vraagt.
- `build/` en `.pixi/` staan niet in Git. `pixi.lock` staat wel in Git.

## Valkuilen

- Gebruik de compilers met het conda-voorvoegsel (`x86_64-conda-linux-gnu-g++`,
  `x86_64-conda-linux-gnu-clang++`). Een kale `clang++` kan de libstdc++ van het systeem pakken.
  De presets regelen dit.
- ASan heeft `llvm-symbolizer` nodig voor regelnummers in de meldingen van Clang. Pixi
  installeert het via `llvm-tools`.
- Na een wijziging in `CMakePresets.json` of in een cache-variabele: `pixi run clean`.
- Een vangnet dat met `abort()` stopt (de debug-modus van libstdc++), telt in CTest altijd als
  fout, ook als `PASS_REGULAR_EXPRESSION` past. Draai zo'n demo via `sh -c "... || true"`.
  ASan stopt met exitcode 1 en heeft dat niet nodig. Voorbeeld: `experimenten/e002-*/CMakeLists.txt`.
- Een demo met `-D_GLIBCXX_DEBUG` mag niet linken met code die zonder die vlag is gebouwd. De
  opbouw van `std::vector` verschilt dan.
- Ninja bouwt alleen wat veranderd is. Een tweede bouw toont dus de waarschuwingen niet meer.
  `pixi run audit` bouwt daarom altijd opnieuw (`--clean-first`).
- cppcheck houdt geen rekening met `-isystem`. De taak `cppcheck` slaat `vreemd/` daarom over
  met `--suppress`. Een padpatroon voor `--suppress` begint zonder `/`: `'*:*vreemd/*'`.
