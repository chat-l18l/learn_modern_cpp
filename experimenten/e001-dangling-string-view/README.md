# e001 — Hangende `std::string_view` naar een tijdelijke `std::string`

**Catalogus:** G01 · **Status:** onderzocht · **Datum:** 2026-11-22

## Vraag

Een functie neemt een `std::string_view` en geeft een `std::string_view` terug die in de invoer
wijst. De aanroeper geeft een tijdelijke `std::string` mee. Welk vangnet vindt dat de view daarna
naar vrijgegeven geheugen wijst?

## Achtergrond

`std::string_view` (C++17) is een pointer plus een lengte. De view is geen eigenaar van de
tekens. Een tijdelijk object leeft tot het einde van de volledige expressie, meestal de
puntkomma. Daarna wijst de view naar geheugen dat niet meer bij een object hoort. Lezen via de
view is ongedefinieerd gedrag.

```cpp
std::string_view word = first_word(make_greeting());  // temporary dies at the ;
use(word);                                            // undefined behaviour
```

Twintig jaar geleden schreef je hier `const char*` en `std::string::c_str()`. Het probleem is
hetzelfde. Het verschil: `std::string_view` ziet eruit als een waarde, en daardoor valt het
minder op.

Een `std::string` met weinig tekens bewaart de tekens in het object zelf (small-string
optimisation, in libstdc++ tot 15 tekens). Een langere `std::string` bewaart de tekens op de
heap. Daarom zijn er twee varianten.

## Verwachting

1. Geen compiler waarschuwt zonder hulp.
2. Clang waarschuwt als de parameter `[[clang::lifetimebound]]` heeft.
3. ASan vindt de fout bij de lange string (heap) en bij de korte string (stack).
4. Zonder sanitizer drukt het programma soms het goede woord af.

## Opzet

| Bestand | Inhoud |
| --- | --- |
| `first_word.hpp/.cpp` | `first_word`, `first_word_annotated` (met `lifetimebound`), `first_word_copy` (geeft een eigen `std::string` terug) |
| `first_word_test.cpp` | Unit tests van de veilige vormen |
| `demo_unsafe.cpp` | De onveilige aanroep, in drie varianten |

| Variant | Aanroep | Tekens staan |
| --- | --- | --- |
| `plain` | `first_word(make_greeting())` | op de heap (51 tekens) |
| `annotated` | `first_word_annotated(make_greeting())` | op de heap |
| `short` | `first_word(make_short_greeting())` | in het object, op de stack (8 tekens) |

## Uitvoeren

```bash
pixi run check
```

De demo met de hand, per preset en variant:

```bash
pixi run build gcc-debug
```

```bash
build/gcc-debug/experimenten/e001-dangling-string-view/e001_demo_unsafe short
```

```bash
pixi run build clang-asan
```

```bash
build/clang-asan/experimenten/e001-dangling-string-view/e001_demo_unsafe plain
```

## Resultaat

Gemeten op 2026-11-22 met GCC 15.3.0, Clang 23.1.6, clang-tidy 23.1.6 en cppcheck 2.22.

| Vangnet | `plain` | `annotated` | `short` |
| --- | --- | --- | --- |
| GCC 15.3, alle waarschuwingen uit `cmake/leren.cmake` | niets | niets | niets |
| Clang 23.1, alle waarschuwingen uit `cmake/leren.cmake` | niets | `-Wdangling-assignment-gsl` en `-Wlifetime-safety-use-after-scope` | niets |
| clang-tidy, configuratie uit `.clang-tidy` | niets | niets | niets |
| cppcheck 2.22 | niets | niets | niets |
| Unit tests (`e001_test`) | ziet de demo niet | ziet de demo niet | ziet de demo niet |
| ASan, GCC en Clang | `heap-use-after-free` | `heap-use-after-free` | `stack-use-after-scope` |

Zonder sanitizer:

| Preset | `plain` | `annotated` | `short` |
| --- | --- | --- | --- |
| `gcc-debug` | onleesbare tekens, exit 1 | onleesbare tekens, exit 1 | `hi`, exit 0 |
| `gcc-release` | onleesbare tekens, exit 1 | onleesbare tekens, exit 1 | `hi`, exit 0 |
| `clang-debug` | onleesbare tekens, exit 1 | onleesbare tekens, exit 1 | `hi`, exit 0 |

De onleesbare tekens bij `plain` en `annotated` zijn waarschijnlijk de beheergegevens die de
geheugenbeheerder van glibc in het vrijgegeven blok schrijft. Dat is niet nagemeten.

Melding van ASan bij `plain`, ingekort:

```text
ERROR: AddressSanitizer: heap-use-after-free on address 0x...
READ of size 38 at 0x... thread T0
    #5 in main experimenten/e001-dangling-string-view/demo_unsafe.cpp:38:17
freed by thread T0 here:
    #6 in std::__cxx11::basic_string<...>::~basic_string()
```

ASan meldt regel 38, de vergelijking `word == expected`. De `printf` op regel 37 leest
hetzelfde vrijgegeven geheugen. ASan meldt die regel niet. De uitvoer van de `printf` verschijnt
ook niet: ASan stopt het programma voordat de buffer van `stdout` wordt geleegd.

## Conclusie

1. Verwachting 1 klopt. Zonder annotatie waarschuwt geen compiler, ook niet met een brede set
   waarschuwingen.
2. Verwachting 2 klopt. Met `[[clang::lifetimebound]]` waarschuwt Clang 23 twee keer. GCC 15 kent
   het attribuut niet.
3. Verwachting 3 klopt. ASan vangt alle drie de varianten, met beide compilers.
4. Verwachting 4 klopt, en het is de belangrijkste les. De korte string geeft zonder sanitizer
   het goede antwoord. Een test die alleen de uitkomst controleert, is groen. De fout is er wel.
5. clang-tidy en cppcheck vinden deze fout niet met de huidige configuratie. clang-tidy stelt
   met `modernize-use-string-view` wel voor dat `make_greeting` een `std::string_view`
   teruggeeft. Dan verdwijnt de fout, maar alleen omdat een letterlijke tekst altijd blijft
   bestaan. Een string die in het programma wordt opgebouwd, blijft het probleem houden. In
   `first_word.cpp` staat daarom een `NOLINT` met de reden.
6. ASan meldt de eerste toegang die het controleert, niet altijd de eerste toegang die fout is.

## Herkennen in een code-review

| Signaal | Waarom verdacht |
| --- | --- |
| Een functie geeft `std::string_view`, `std::span`, `T*` of `const T&` terug, en neemt een parameter van een vergelijkbaar type | De uitkomst leent waarschijnlijk van de parameter |
| De aanroep krijgt een functie-aanroep als argument: `f(g())` | Het argument is een tijdelijk object |
| Een `std::string_view` als lid van een klasse of als lokale variabele die langer leeft dan de regel | Wie is de eigenaar, en leeft die lang genoeg? |
| `auto x = ...` waar de rechterkant een view teruggeeft | `auto` verbergt dat `x` een view is |

## Veilig alternatief

| Situatie | Alternatief |
| --- | --- |
| De aanroeper heeft de eigenaar | Bewaar de `std::string` in een variabele, en maak daarna de view (`SafeWhenCallerKeepsTheOwner`) |
| Het resultaat moet langer leven dan de invoer | Geef een `std::string` terug (`first_word_copy`) |
| Een eigen API die leent | Zet `[[clang::lifetimebound]]` op de parameter, zodat Clang een tijdelijk object meldt |
| Altijd | Draai de tests ook onder `gcc-asan` of `clang-asan` |

## Bronnen

- cppreference: [`std::basic_string_view`](https://en.cppreference.com/w/cpp/string/basic_string_view),
  onderdeel "Notes" over levensduur.
- Clang: [`lifetimebound`](https://clang.llvm.org/docs/AttributeReference.html#lifetimebound).
- AddressSanitizer: [documentatie](https://github.com/google/sanitizers/wiki/AddressSanitizer).
