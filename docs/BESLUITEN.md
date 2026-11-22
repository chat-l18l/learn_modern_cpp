# Besluiten

Dit document houdt de keuzes bij die de werkplaats vormen. Een **Besluit** is genomen. Een
**Voorstel** wacht op akkoord. Een besluit dat later verandert, blijft staan. Het nieuwe besluit
verwijst naar het oude.

## Overzicht

| ID | Onderwerp | Status | Datum |
| --- | --- | --- | --- |
| B01 | Bouwomgeving met pixi en conda-forge | Besluit | 2026-11-22 |
| B02 | CMake met Ninja en presets | Besluit | 2026-11-22 |
| B03 | GoogleTest als testraamwerk | Besluit | 2026-11-22 |
| B04 | C++23 zonder compiler-extensies | Besluit | 2026-11-22 |
| B05 | Twee compilers: GCC en Clang | Besluit | 2026-11-22 |
| B06 | Sanitizers als vaste presets, met vangnet-tests | Besluit | 2026-11-22 |
| B07 | Geen `-Werror`; waarschuwingen zijn meetgegevens | Herzien door B12 | 2026-11-22 |
| B08 | Taal: documenten Nederlands, code Engels | Besluit | 2026-11-22 |
| B09 | Geen C++-modules | Besluit | 2026-11-22 |
| B10 | Opmaak van de code | Voorstel | 2026-11-22 |
| B11 | Stijl van commitberichten | Voorstel | 2026-11-22 |
| B12 | `-Werror` voor de veilige code (herziet B07) | Besluit | 2026-11-22 |
| B13 | Extra waarschuwingen: `-Wcast-align`, `-Wdouble-promotion` | Besluit | 2026-11-22 |
| B14 | Fase 5: code van anderen eerst controleren, dan pas gebruiken | Besluit | 2026-11-22 |
| B15 | Eenvoud boven slimheid: regels voor templates, overloads, opbouw en commentaar | Besluit | 2026-11-22 |
| B16 | Stackbudget per functie met `-Wframe-larger-than` | Besluit | 2026-11-22 |
| B17 | De aanroeper alloceert; rekenwerk in pure functies | Besluit | 2026-11-22 |
| B18 | Ontwerpprincipes en auditvragen als eigen documenten | Besluit | 2026-11-22 |

---

### B01 — Bouwomgeving met pixi en conda-forge

**Besluit:** pixi installeert alle gereedschap uit conda-forge. `pixi.lock` legt de exacte
versies vast en staat in Git.

**Waarom:** de omgeving is per project en reproduceerbaar. Er is geen `sudo` nodig. De
systeemcompiler (GCC 13 van Ubuntu 24.04) is ouder dan GCC 15 uit conda-forge.

**Gevolg:** draai alles via `pixi run`. Een nieuwe versie van het gereedschap is een bewuste
stap: `pixi update`, daarna `pixi run check`, daarna een commit met de nieuwe `pixi.lock`.

### B02 — CMake met Ninja en presets

**Besluit:** CMake met de Ninja-generator. Elke combinatie van compiler en vangnet is een
preset in `CMakePresets.json`. Elke preset bouwt in zijn eigen map `build/<preset>/`.

**Waarom:** CMake is de standaard voor C++. Presets maken de combinaties benoembaar en
herhaalbaar. Aparte mappen maken vergelijken tussen presets eenvoudig.

### B03 — GoogleTest als testraamwerk

**Besluit:** GoogleTest 1.18 uit conda-forge, met `gtest_discover_tests`, zodat elke `TEST()`
een eigen CTest-test is.

**Waarom:** GoogleTest is het meest gebruikte raamwerk. Het heeft death tests
(`EXPECT_DEATH`), die bruikbaar zijn om een crash als verwachte uitkomst te testen. Catch2 heeft
geen death tests.

### B04 — C++23 zonder compiler-extensies

**Besluit:** `CMAKE_CXX_STANDARD 23`, `CMAKE_CXX_EXTENSIONS OFF`.

**Waarom:** C++23 is de nieuwste standaard die GCC 15 en Clang 23 goed ondersteunen. Een
experiment mag een ouder niveau kiezen als de vergelijking het onderwerp is (zie G04).

### B05 — Twee compilers: GCC en Clang

**Besluit:** elke experiment bouwt met GCC 15 en met Clang 23. De presets gebruiken de
conda-namen `x86_64-conda-linux-gnu-g++` en `x86_64-conda-linux-gnu-clang++`.

**Waarom:** de twee compilers waarschuwen voor verschillende fouten. In e001 waarschuwt Clang
wel en GCC niet. De conda-naam van Clang gebruikt de libstdc++ 15 uit de pixi-omgeving. Een
kale `clang++` kan de libstdc++ van het systeem gebruiken. Dan past GoogleTest er niet meer bij.

### B06 — Sanitizers als vaste presets, met vangnet-tests

**Besluit:** de presets `gcc-asan` en `clang-asan` bouwen met
`-fsanitize=address,undefined -fno-sanitize-recover=all`. De preset `clang-tsan` bouwt met
ThreadSanitizer. Een vangnet-test registreert de onveilige demo en slaagt alleen als de
sanitizer de verwachte melding geeft (`PASS_REGULAR_EXPRESSION`).

**Waarom:** een vangnet-test maakt de bewering "ASan vangt dit" controleerbaar. Als een nieuwe
versie van de compiler de fout niet meer vangt, faalt de test.

**Gevolg:** zonder sanitizer draait de demo niet als test. De uitkomst van ongedefinieerd gedrag
is geen betrouwbare testuitkomst.

### B07 — Geen `-Werror`; waarschuwingen zijn meetgegevens

**Besluit:** de waarschuwingen in `cmake/leren.cmake` staan aan, maar een waarschuwing stopt de
build niet.

**Waarom:** de onveilige demo's moeten bouwen. De waarschuwing die een demo oplevert, is juist
wat we meten. De veilige code moet wel zonder waarschuwingen bouwen. Dat controleren we bij elke
stap met het oog.

**Herzien op 2026-11-22 door B12:** de veilige code bouwt nu met `-Werror`. Voor de demo's
blijft B07 gelden.

### B08 — Taal: documenten Nederlands, code Engels

**Besluit:** README, verslagen en documenten in `docs/` in het Nederlands, volgens de principes
van Simplified Technical English. Code, namen en commentaar in de code in het Engels.

**Waarom:** de documenten zijn voor de eigen studie. Engelse code past bij de meldingen van
compilers en sanitizers, en bij voorbeelden op internet.

### B09 — Geen C++-modules

**Besluit:** `CMAKE_CXX_SCAN_FOR_MODULES OFF`.

**Waarom:** met C++23 bouwt CMake 4.5 anders de std-module voor elk doel. Dat kost bouwtijd en
gaf tientallen `-Wundef`-waarschuwingen uit de headers van libstdc++. Modules zijn geen onderwerp
van de eerste fasen.

### B10 — Opmaak van de code

**Voorstel:** `.clang-format` op basis van LLVM, met 4 spaties inspringen, regels tot 100 tekens
en de accolade van een functie op een eigen regel.

**Alternatief:** de stijl van Pieter Hintjens (CZMQ), die in de C-projecten wordt gebruikt: het
returntype op een eigen regel en een spatie voor het haakje van een aanroep (`foo (x)`). In C++
geldt die spatie dan ook voor `std::move (x)` en voor lambda's. Dat ziet er in C++ ongewoon uit
en wijkt af van bijna alle voorbeelden.

**Te beslissen:** het voorstel overnemen, of de stijl van Hintjens ook voor C++ gebruiken.

### B11 — Stijl van commitberichten

**Voorstel:** Nederlandse onderwerpregel als hele zin die zegt wat er nu waar is, zoals in
Energy Owl. Met het experiment- of catalogus-ID als dat past. Voorbeeld:
`e002: iterator ongeldig na push_back, ASan vangt het (G02)`.

**Alternatief:** Conventional Commits zoals in Omnibot: `feat(e002): ...`, `docs: ...`.

**Te beslissen:** het voorstel overnemen, of Conventional Commits gebruiken.

### B12 — `-Werror` voor de veilige code (herziet B07)

**Besluit (2026-11-22):** `leren_target()` krijgt de optie `UNSAFE`. Een doel zonder `UNSAFE` bouwt met
`-Werror`. Alleen de demo's krijgen `UNSAFE` en bouwen zonder `-Werror`.

**Waarom:** B07 zegt dat de veilige code zonder waarschuwingen moet bouwen, maar niets dwingt dat
af. Een nieuwe waarschuwing in de veilige code valt nu alleen op als iemand de uitvoer leest. Met
`-Werror` stopt de build.

**Gevolg:** een vals alarm in de veilige code (zoals in e002) moet je onderdrukken, met de reden
erbij. Dat is al de werkwijze. De preset `clang-audit` gebruikt nooit `-Werror`: een audit is
een rapport en moet alle meldingen tonen.

**Uitgevoerd:** `leren_target(<doel> [UNSAFE])` in `cmake/leren.cmake`. Bewezen met een
geplante ongebruikte variabele in `grow.cpp`: de bouw stopt met `-Werror=unused-variable`.

### B13 — Extra waarschuwingen: `-Wcast-align`, `-Wdouble-promotion`

**Besluit (2026-11-22):** voeg `-Wcast-align` en `-Wdouble-promotion` toe aan `cmake/leren.cmake`.

**Waarom:** beide staan in een richtlijn voor agents die we hebben bekeken, en ontbreken bij ons.
`-Wcast-align` meldt een cast naar een type met een strengere uitlijning. `-Wdouble-promotion`
meldt een `float` die stil een `double` wordt.

**Niet overnemen uit die richtlijn:**

| Vlag | Waarom niet |
| --- | --- |
| `-Wlifetime` | Bestaat niet. GCC 15 stopt met `unrecognized command-line option`. Clang 23 geeft `unknown warning option`. Gemeten op 2026-11-22. De Clang-vlag die wel bestaat, is `-Wlifetime-safety`; die staat al aan. |
| `-Wunused` | Zit al in `-Wall` |
| `-fsanitize=...` zonder `-fno-sanitize-recover=all` | Dan meldt UBSan de fout, maar het programma eindigt met exitcode 0 en de test is groen. Gemeten op 2026-11-22 met een signed overflow. Wij hebben `-fno-sanitize-recover=all` al (B06). |

### B14 — Fase 5: code van anderen eerst controleren, dan pas gebruiken

**Besluit (2026-11-22):** een nieuwe fase in [PLAN.md](PLAN.md) met een audit-preset en experimenten
over code van anderen. Pas na die fase komt er een protocol voor code van anderen in `AGENTS.md`.

**Waarom:** drie voorlopige metingen op 2026-11-22 laten zien dat de gewone opzet fouten in code
van anderen niet ziet:

| Meting | Uitkomst |
| --- | --- |
| Een header met een niet-geïnitialiseerde variabele en een stille versmalling, ingevoegd met `-I` | GCC: 2 waarschuwingen. Clang: 4 waarschuwingen. |
| Dezelfde header, ingevoegd met `-isystem` | GCC en Clang: **0 waarschuwingen** |
| Een functie die buiten een array leest, in een bibliotheek die **zonder** ASan is gebouwd, aangeroepen uit een programma **met** ASan | **Geen melding.** Het programma drukt een willekeurige waarde af. |
| Dezelfde functie, bibliotheek **met** ASan gebouwd | `heap-buffer-overflow` |
| Clang `-Wunsafe-buffer-usage` op dezelfde header | Meldt de pointer-index `p[i]` tijdens het compileren |

CMake voegt de headers van een geïmporteerde bibliotheek standaard in als `-isystem`. Elke
bibliotheek uit pixi, conda of `find_package` is dus stil voor de compiler. Een bibliotheek uit
pixi of conda is ook al gebouwd, dus zonder ASan.

**Wat de fase moet opleveren:**

1. Een preset `clang-audit`: code van anderen als gewone bron (niet `-isystem`), met onze
   waarschuwingen plus `-Wunsafe-buffer-usage`, en clang-tidy met een `HeaderFilterRegex` die
   de code van anderen omvat.
2. De regel: code van anderen bouwt uit de bron mee in de presets met een sanitizer. Een
   bibliotheek die alleen als binair bestand bestaat, staat in het verslag als blinde vlek.
3. Een RAII-wrapper aan de grens: pointer plus lengte wordt `std::span`, eigendom wordt
   `std::unique_ptr` met een eigen deleter, een foutcode wordt `std::expected`. De unit tests en
   de vangnet-tests richten zich op de wrapper.
4. Een fuzz-test (libFuzzer, `-fsanitize=fuzzer`) voor elke functie van anderen die invoer van
   buiten leest.

**Uitgevoerd op 2026-11-22:** punt 1 (preset `clang-audit`, taak `pixi run audit`) en punt 2
(de doelen `minilib` en `minilib_prebuilt`). Punt 3 in eerste vorm in e003. De metingen staan in
[e003](../experimenten/e003-code-van-anderen/README.md). Open: punt 4, en het protocol in
`AGENTS.md`.

### B15 — Eenvoud boven slimheid

**Besluit (2026-11-22):** de regels in `AGENTS.md`, paragraaf "Eenvoud boven slimheid". De
bron is een gevonden richtlijn voor agents. **Verplaatst op 2026-11-22** naar
[PRINCIPES.md](PRINCIPES.md), P3 en P4 (B18). Vier regels zijn overgenomen. Twee regels zijn
aangepast.

| Regel uit de richtlijn | Wat we ermee doen |
| --- | --- |
| Templates alleen voor 3+ types, altijd met `concepts` | Overgenomen |
| Geen overloads, unieke beschrijvende namen | Overgenomen, met uitzonderingen voor constructors, operators en namen uit de standaardbibliotheek. De voorbeelden in `camelCase` (`parseFromFile`) worden `snake_case`. |
| Gedrag bij elkaar houden | Overgenomen, met een criterium voor splitsen: een eigen naam of een eigen test |
| Structs voor data, vrije functies, `virtual` alleen voor runtime-polymorfie | Overgenomen, en aangevuld: een gesloten verzameling types is een `enum class` of `std::variant`; een klasse met `virtual` heeft een `virtual` destructor en gaat nooit by value |
| Commentaar voor het waarom | Aangepast: ook het contract (eigenaar, levensduur, voorwaarden) hoort in commentaar. In demo's mag commentaar het mechanisme uitleggen. |

**Waarom:** eenvoudige code geeft minder plek aan de fouten uit de catalogus, en een review
ziet een fout sneller. De regel over overloads is gemeten: zie G27.

**Wat geen hulpmiddel afdwingt:** de grens van drie types, de lengte van een functie, en de
keuze tussen `virtual` en een `enum class`. Die regels bewaakt de review. De controles die wel
bestaan, staan al aan via `.clang-tidy` en `cmake/leren.cmake`.

### B16 — Stackbudget per functie met `-Wframe-larger-than`

**Besluit (2026-11-22):** elk doel zonder `UNSAFE` en zonder `HOST` bouwt met
`-Wframe-larger-than=${LEREN_FRAME_LIMIT}`, en dus met `-Werror`. `LEREN_FRAME_LIMIT` is een
cache-variabele in `cmake/leren.cmake`, standaard **512 bytes**. De grens geldt alleen in de
presets zonder sanitizer.

**Waarom:** het ontwerpdoel is een stack van 4 KiB per taak. Met 512 bytes per functie past een
keten van acht functies diep. e004 laat zien dat een capture by value van 4 KiB het stackframe
verdubbelt, zonder dat de code dat laat zien. `-Wframe-larger-than` meldt dat tijdens het
compileren, met GCC en met Clang.

**Gemeten op 2026-11-22:**

| Meting | Uitkomst |
| --- | --- |
| Grootste stackframe in de bibliotheekcode van e001–e004, `gcc-debug` | 128 bytes (`e004::counter_table_add`) |
| Een geplante lokale `std::array<char, 600>` in `counter_tick` | De bouw stopt: `frame size of 624 bytes is larger than 512 bytes` |
| Testbestanden, `gcc-debug` | 1120–1600 bytes in `__static_initialization_and_destruction_0`: de registratie van de tests door GoogleTest |
| `std::vector<int>::_M_realloc_append` onder `clang-asan` | 536 bytes. Zonder ASan binnen 512. |

**Gevolg:**

- Tests krijgen de markering `HOST` en vallen buiten het budget. Ze draaien niet op het apparaat.
- Het budget geldt niet in de presets met een sanitizer.
- Het budget geldt per functie. Het totaal langs de diepste keten van aanroepen vraagt
  `-fcallgraph-info=su` (GCC). Dat is nog niet gemeten.
- Meet het echte budget met de compiler en de optimalisatie van het apparaat. Onder `-O2`
  verdwenen in e004 hele buffers.

### B17 — De aanroeper alloceert; rekenwerk in pure functies

**Besluit (2026-11-22):** de regels in `AGENTS.md`, paragraaf "Geheugen en stack: de
aanroeper alloceert". **Verplaatst op 2026-11-22** naar [PRINCIPES.md](PRINCIPES.md), P1 en P5
(B18).

1. Een functie alloceert geen buffers en geeft geen eigen container terug. De aanroeper
   alloceert en geeft de buffer mee als `std::span`.
2. Grote gegevens staan in `static` opslag of in een struct die de eigenaar is.
3. Rekenwerk staat in pure functies. Invoer en uitvoer staan in weinig, duidelijk benoemde
   functies.

**Waarom:** het geheugengebruik is dan vooraf bekend en zichtbaar in de code. Er is geen
verborgen heap-allocatie (zoals `std::function` in e004), en geen grote kopie op de stack. Een
pure functie is eenvoudig te testen: dezelfde invoer geeft dezelfde uitkomst.

**Gevolg:** de code in e001–e004 volgt deze regel niet overal. Die code blijft staan als
studiemateriaal. Nieuwe code volgt de regel.

### B18 — Ontwerpprincipes en auditvragen als eigen documenten

**Besluit (2026-11-22):** het project heeft twee doelen: leren hoe C++ misgaat, en
ontwerpprincipes vastleggen als regels die je kunt toetsen. De principes staan in
[PRINCIPES.md](PRINCIPES.md) (P1–P6), de vragen voor een audit in
[AUDITVRAGEN.md](AUDITVRAGEN.md) (A1–A16). De regeltabellen uit `AGENTS.md` (B15, B17) zijn
daarheen verplaatst. `AGENTS.md` verwijst ernaar.

**Waarom:** de regels stonden verspreid over besluiten en een bestand voor agents. Daar zijn ze
niet mee te nemen naar andere projecten, en niet te gebruiken bij een audit. In een eigen
document heeft elk principe een regel, een reden, een toets en bewijs.

**Gevolg:**

1. Een principe is "vastgesteld" als het een toets heeft en minstens één verwijzing naar bewijs.
   Anders is het een "werkhypothese". Op 2026-11-22: P1, P2, P5 en P6 vastgesteld; P3 en P4
   werkhypothese.
2. Na elk experiment volgt de vraag: bevestigt, verandert of weerlegt het een principe?
3. Fase 6 van het [plan](PLAN.md) toetst de principes aan code uit de praktijk.
