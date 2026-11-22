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
| B07 | Geen `-Werror`; waarschuwingen zijn meetgegevens | Besluit | 2026-11-22 |
| B08 | Taal: documenten Nederlands, code Engels | Besluit | 2026-11-22 |
| B09 | Geen C++-modules | Besluit | 2026-11-22 |
| B10 | Opmaak van de code | Voorstel | 2026-11-22 |
| B11 | Stijl van commitberichten | Voorstel | 2026-11-22 |

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
