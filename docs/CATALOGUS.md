# Catalogus van onveilige constructies

Deze catalogus is de werklijst. Elke regel is een constructie die nog steeds geldig C++ is, maar
die ongedefinieerd gedrag geeft of die een gewone test niet ziet. We werken de lijst stap voor
stap af. Een afgeronde regel blijft staan, met een verwijzing naar het experiment.

## Status

| Status | Betekenis |
| --- | --- |
| open | Nog niet onderzocht |
| bezig | Er loopt een experiment |
| onderzocht | Het verslag staat er, met gemeten uitkomsten |

De kolom "Verwacht vangnet" is een verwachting. De kolom "Gemeten" bevat alleen wat een
experiment heeft laten zien.

## Fase 1 — Levensduur van objecten

| ID | Constructie | Waarom een test het mist | Verwacht vangnet | Gemeten | Status |
| --- | --- | --- | --- | --- | --- |
| G01 | `std::string_view` naar een tijdelijke `std::string` | Het vrijgegeven geheugen bevat vaak nog de oude tekens | ASan; Clang met `lifetimebound` | Zie [e001](../experimenten/e001-dangling-string-view/README.md): ASan vangt alle drie de varianten. Clang waarschuwt alleen met `[[clang::lifetimebound]]`. GCC 15.3, clang-tidy en cppcheck 2.22 melden niets. | onderzocht |
| G02 | Iterator, pointer of referentie in een `std::vector` na `push_back` | Zonder herallocatie gaat het goed. De fout verschijnt pas bij een bepaalde grootte. | ASan; `_GLIBCXX_DEBUG`; cppcheck | Zie [e002](../experimenten/e002-vector-push-back/README.md): ASan vangt alle drie de varianten. cppcheck 2.22 en Clang 23 (`-Wlifetime-safety-invalidation`) ook, maar beide melden ook correcte `reserve`-code. De debug-modus vangt iterators, geen referenties. GCC en clang-tidy melden niets. | onderzocht |
| G03 | Lambda die een lokale variabele by reference vangt en langer leeft dan die variabele | De lambda draait vaak direct, binnen de scope | ASan (stack-use-after-return) | Zie [e004](../experimenten/e004-lambda-capture/README.md): ASan, cppcheck en clang-tidy vangen beide varianten. Clang 23 ook. GCC 15 meldt alleen `[=]` met `this`. Zonder sanitizer geeft `gcc-debug` het juiste antwoord. | onderzocht |
| G04 | Range-for over een deel van een tijdelijk object: `for (auto x : make().items())` | In C++23 is het veilig (P2718), in C++20 niet. Dezelfde code, twee uitkomsten. | ASan; vergelijk `-std=c++20` met `-std=c++23` | | open |
| G05 | Gebruik van een object na `std::move` | Een verplaatst object is "geldig maar onbepaald". Vaak is het leeg en lijkt het te werken. | clang-tidy `bugprone-use-after-move` | | open |
| G06 | Functie die `const T&` teruggeeft, aangeroepen met een tijdelijk object: `const auto& m = std::min(a, f());` | Het tijdelijke object leeft tot het einde van de regel | GCC `-Wdangling-reference`; ASan | | open |
| G07 | `std::span` naar een `std::vector` die daarna groeit of verdwijnt | Zelfde oorzaak als G01 en G02, ander type | ASan | | open |

## Fase 2 — Rekenen en geheugen

| ID | Constructie | Waarom een test het mist | Verwacht vangnet | Gemeten | Status |
| --- | --- | --- | --- | --- | --- |
| G08 | Overflow van een signed integer | Bij `-O0` loopt de waarde rond. Bij `-O2` mag de compiler aannemen dat overflow niet bestaat. | UBSan; vergelijk `gcc-debug` met `gcc-release` | | open |
| G09 | Lezen van een niet-geïnitialiseerde variabele | De stack bevat toevallig een bruikbare waarde | `-Wuninitialized`; MemorySanitizer; Valgrind | | open |
| G10 | Index buiten de grenzen met `operator[]` | Het geheugen ernaast is vaak leesbaar | ASan; `_GLIBCXX_ASSERTIONS` | | open |
| G11 | Type punning met `reinterpret_cast` (strict aliasing) | Werkt bij `-O0`, breekt bij `-O2` | `-Wstrict-aliasing`; veilig: `std::bit_cast` | | open |
| G12 | Vergelijking tussen signed en unsigned, impliciete versmalling | De testwaarden zijn klein en positief | `-Wsign-compare`, `-Wconversion` | | open |
| G13 | `*opt` op een lege `std::optional`, `*exp` op een `std::expected` met fout | De test gebruikt alleen het goede pad | `_GLIBCXX_ASSERTIONS` | | open |

## Fase 3 — Gelijktijdigheid

| ID | Constructie | Waarom een test het mist | Verwacht vangnet | Gemeten | Status |
| --- | --- | --- | --- | --- | --- |
| G14 | Data race op een gewone variabele | De race treedt zelden op en hangt af van de timing | TSan | | open |
| G15 | Twee mutexen in verschillende volgorde (deadlock) | Een deadlock vraagt een bepaalde volgorde van threads | TSan (lock-order-inversion) | | open |
| G16 | `std::thread` die verdwijnt zonder `join()` | Het programma stopt direct met `std::terminate` — dit is eerder te vinden. Veilig: `std::jthread`. | Test die crasht | | open |

## Fase 4 — Opbouw van het programma

| ID | Constructie | Waarom een test het mist | Verwacht vangnet | Gemeten | Status |
| --- | --- | --- | --- | --- | --- |
| G17 | Object slicing: een afgeleid object by value als basisklasse doorgeven | De code compileert en de basisfunctie geeft een plausibel antwoord | clang-tidy `cppcoreguidelines-slicing` | | open |
| G18 | Volgorde van initialisatie van globale objecten in verschillende bestanden | Hangt af van de volgorde van linken | ASan `check_initialization_order` | | open |
| G19 | Twee verschillende definities van dezelfde naam (ODR-schending) | De linker kiest er één, zonder melding | ASan `detect_odr_violation`; `-flto -Wodr` | | open |
| G20 | Cyclus van `std::shared_ptr` (geheugenlek) | Een lek geeft geen fout | LeakSanitizer (deel van ASan) | | open |
| G21 | Exceptie uit een destructor | Een destructor is standaard `noexcept`: het programma stopt | Test die crasht | | open |
| G27 | Overload die een andere versie kiest dan de lezer verwacht: `log_it("hello")` met `log_it(bool)` en `log_it(const std::string &)` | De code compileert en doet iets. Alleen een test op precies die aanroep ziet het verschil. | Clang `-Wstring-conversion`; clang-tidy `readability-implicit-bool-conversion` | Voorlopig (2026-11-22): de `bool`-versie wint. GCC 15 meldt niets. Clang en clang-tidy melden het. `set(NULL)` met `set(int)` en `set(void *)` compileert niet: dubbelzinnig. Zie B15. | open |

## Fase 5 — Code van anderen en het gereedschap

| ID | Constructie | Waarom een test het mist | Verwacht vangnet | Gemeten | Status |
| --- | --- | --- | --- | --- | --- |
| G22 | Rauwe `new` en `delete`, of een rauwe pointer die eigenaar is | Een lek of dubbele `delete` treedt alleen op bij een foutpad | clang-tidy `cppcoreguidelines-owning-memory`; LeakSanitizer | | open |
| G23 | `std::string_view` of `std::span` als lid van een klasse | Het object leeft langer dan de tekst waar het lid naar wijst | Clang `lifetimebound`; ASan | | open |
| G24 | Waarschuwingen in headers van anderen, verborgen door `-isystem` | De compiler meldt niets. Er is geen test die faalt. | Audit-preset zonder `-isystem` | Zie [e003](../experimenten/e003-code-van-anderen/README.md): de gewone bouw geeft 0 waarschuwingen, ook met `-Werror`. `clang-audit` vindt de niet-geïnitialiseerde variabele en de versmalling. cppcheck negeert `-isystem`. | onderzocht |
| G25 | Een bibliotheek die zonder sanitizer is gebouwd | ASan ziet geen fout in code die het niet heeft geïnstrumenteerd | Bibliotheek uit de bron bouwen in de sanitizer-preset | Zie [e003](../experimenten/e003-code-van-anderen/README.md): ASan ziet het lezen buiten de array niet in de al gebouwde bibliotheek, wel in de bibliotheek uit de bron. Een `inline`-functie uit de header ziet ASan altijd. | onderzocht |
| G26 | Pointer-rekenen en indexeren via een rauwe pointer in code van anderen | Het geheugen ernaast is vaak leesbaar | Clang `-Wunsafe-buffer-usage` | Zie [e003](../experimenten/e003-code-van-anderen/README.md): meldt elke index via een rauwe pointer, ook de foute. Wijst plekken aan, bewijst geen fout. Geen meldingen in onze veilige code. | onderzocht |
| G28 | Een index die naar een slot wijst dat opnieuw is gebruikt (verouderde index) | Het geheugen is geldig. De code leest het verkeerde element, en geen sanitizer ziet het. | Handle met generatie (P2 regel 8); vaste tabellen zonder verwijderen | | open |
| G29 | Verwisselde parameters van hetzelfde kale type: `set_limits(int low_mv, int high_mv)` aangeroepen als `set_limits(high, low)`, of met een temperatuur | De code compileert. Een test met symmetrische waarden ziet het niet. | Sterke types (P2 regels 6 en 10); clang-tidy `bugprone-easily-swappable-parameters` | | open |
