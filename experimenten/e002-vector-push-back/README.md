# e002 — Referentie in een `std::vector` na `push_back`

**Catalogus:** G02 · **Status:** onderzocht · **Datum:** 2026-11-22

## Vraag

Code bewaart een referentie, iterator of pointer naar een element van een `std::vector`. Daarna
roept de code `push_back` aan. Welk vangnet vindt dat de referentie naar vrijgegeven geheugen
wijst? En wanneer is dezelfde code wel correct?

## Achtergrond

```cpp
std::vector<int> v = {1, 2, 3};
int& first = v[0];
v.push_back(4);
std::cout << first;  // undefined behaviour
```

1. `v` krijgt een blok op de heap. Met libstdc++ is `capacity()` dan 3: precies vol.
2. `first` is een referentie naar het adres van `v[0]` in dat blok.
3. `push_back(4)` past niet. De vector vraagt een groter blok, kopieert `1, 2, 3` naar het nieuwe
   blok, zet `4` erachter en geeft het oude blok vrij.
4. `first` wijst nog naar het oude blok. Lezen via `first` is lezen uit vrijgegeven geheugen.

De standaard zegt: als `push_back` de capaciteit vergroot, worden alle referenties, pointers en
iterators naar elementen ongeldig. Als de capaciteit groot genoeg is, blijven referenties en
pointers naar bestaande elementen geldig. Alleen `end()` wordt dan ongeldig.

Daarom is dit moeilijk te testen. Of de code fout is, hangt af van `size()` en `capacity()` op
het moment van `push_back`. Die waarden staan niet in de broncode. Een test met een andere
grootte ziet de fout niet.

De capaciteit na `{1, 2, 3}` staat niet in de standaard. Dat de capaciteit 3 is, geldt voor
libstdc++ en wordt hieronder gemeten.

## Verwachting

1. GCC en Clang waarschuwen bij geen enkele variant.
2. cppcheck meldt de referentie en de range-for (cppcheck heeft een controle `invalidContainer`).
   clang-tidy meldt niets.
3. ASan meldt `heap-use-after-free` bij de referentie, de iterator en de range-for.
4. De debug-modus van libstdc++ (`-D_GLIBCXX_DEBUG`) vangt de iterator en de range-for, maar
   niet de referentie. De debug-modus controleert iterators, geen referenties.
5. Met `reserve(4)` vooraf is de code correct, en meldt ASan niets.
6. Zonder sanitizer is de uitkomst onvoorspelbaar.

## Opzet

| Bestand | Inhoud |
| --- | --- |
| `grow.hpp/.cpp` | `append_evens`: voegt een kopie van elk even element achteraan toe. De veilige vorm van de range-for. |
| `grow_test.cpp` | Unit tests van de veilige vormen: index, `reserve`, referentie na de wijziging |
| `demo_unsafe.cpp` | De onveilige vormen, en een correcte controlevariant |

| Variant | Code | Ongedefinieerd gedrag |
| --- | --- | --- |
| `reference` | `int& first = v[0]; v.push_back(4); use(first);` | ja |
| `iterator` | `auto it = v.begin(); v.push_back(4); use(*it);` | ja |
| `range-for` | `for (int x : v) if (x % 2 == 0) v.push_back(x);` | ja |
| `spare` | `v.reserve(4);` vooraf, daarna zoals `reference` | nee |

De demo bouwt twee keer: gewoon (`e002_demo_unsafe`) en met de debug-modus van libstdc++
(`e002_demo_unsafe_debugmode`).

## Uitvoeren

```bash
pixi run check
```

De debug-modus zelf draaien:

```bash
pixi run build gcc-debug
```

```bash
build/gcc-debug/experimenten/e002-vector-push-back/e002_demo_unsafe_debugmode iterator
```

De meldingen van cppcheck op de demo zien:

```bash
pixi run cppcheck-unsafe
```

## Resultaat

Gemeten op 2026-11-22 met GCC 15.3.0, Clang 23.1.6, clang-tidy 23.1.6, cppcheck 2.22 en
libstdc++ 15.

Na `std::vector<int> v = {1, 2, 3}` is `capacity()` 3. Na `push_back(4)` is `capacity()` 6:
libstdc++ verdubbelt.

### Vangnetten

| Vangnet | `reference` | `iterator` | `range-for` | `spare` (correct) |
| --- | --- | --- | --- | --- |
| GCC 15.3, waarschuwingen uit `cmake/leren.cmake` | niets | niets | niets | niets |
| Clang 23.1, `-Wlifetime-safety-invalidation` | melding | melding | melding | **melding (vals)** |
| clang-tidy | niets | niets | niets | niets |
| cppcheck 2.22 | `invalidContainerReference` | `invalidContainer` | `invalidContainerLoop` | **`invalidContainer` (vals)** |
| Debug-modus libstdc++ (`_GLIBCXX_DEBUG`) | **niets** | `attempt to dereference a singular iterator` | `attempt to increment a singular iterator` | niets |
| ASan, GCC en Clang | `heap-use-after-free` | `heap-use-after-free` | `heap-use-after-free` | niets |

De demo-variant `spare` deelt zijn code met `reference`. De valse meldingen in de kolom `spare`
zijn daarom gemeten op de unit test `SafeReserve.KeepsTheAddressWhenCapacityIsEnough`. Die test
is correcte code.

Clang meldde ook een valse waarschuwing op de veilige functie `append_evens` in `grow.cpp`: "parameter
'v' is later invalidated", bij `push_back` en daarna `v[i]`. De index-lus maakt bij elke stap een
nieuwe referentie. De melding is dus onterecht. In `grow.cpp` en `grow_test.cpp` staat een
`#pragma` of `cppcheck-suppress` met de reden.

### Zonder sanitizer

| Preset | `reference` | `iterator` | `range-for` | `spare` |
| --- | --- | --- | --- | --- |
| `gcc-debug` | willekeurig getal, exit 1 | willekeurig getal, exit 1 | `1 2 3 2` plus een willekeurig getal, exit 1 | `first=1`, exit 0 |
| `gcc-release` | willekeurig getal, exit 1 | willekeurig getal, exit 1 | **`1 2 3 2`, exit 0** | `first=1`, exit 0 |
| `clang-debug` | willekeurig getal, exit 1 | willekeurig getal, exit 1 | `1 2 3 2` plus een willekeurig getal, exit 1 | `first=1`, exit 0 |

Bij `range-for` in `gcc-debug` leest de lus het derde element uit het vrijgegeven blok. Dat getal
was even, en de lus voegde het toe. In `gcc-release` geeft dezelfde code het juiste antwoord.

## Conclusie

1. Verwachting 1 klopt voor GCC, niet voor Clang. Clang 23 meldt alle drie de fouten met
   `-Wlifetime-safety-invalidation`. Clang meldt ook twee keer correcte code.
2. Verwachting 2 klopt. cppcheck vindt alle drie de fouten, met een duidelijke uitleg. cppcheck
   meldt ook de correcte `reserve`-code. clang-tidy vindt niets.
3. Verwachting 3 klopt. ASan vangt alle drie de fouten, met beide compilers, en is stil bij `spare`.
4. Verwachting 4 klopt. De debug-modus vangt iterators en mist de referentie. Een pointer
   (`v.data()`, `&v[0]`) is voor de debug-modus hetzelfde als een referentie. Dat is niet apart
   gemeten.
5. Verwachting 5 klopt. Met `reserve(4)` is de code correct en ASan is stil. Maar Clang en
   cppcheck kunnen de capaciteit niet zien. Voor hen ziet de correcte code eruit als de fout.
6. Verwachting 6 klopt, en het is de belangrijkste les: `range-for` geeft onder `-O2` het juiste
   antwoord en onder `-O0` niet. Een test die alleen in release draait, is groen.
7. In tegenstelling tot e001 vinden statische hulpmiddelen deze fout wel. Het patroon
   "referentie, dan `push_back`, dan gebruik" staat binnen één functie. In e001 liep de fout via
   een functie-aanroep.

### Over de drie oplossingen uit het idee

| Oplossing | Oordeel |
| --- | --- |
| Een index in plaats van een referentie | Correct, en de hulpmiddelen begrijpen het (met één valse melding van Clang). De index blijft geldig bij `push_back`, maar niet bij `insert` of `erase` vóór die index. |
| `reserve()` vooraf | Correct, maar breekbaar. De juistheid hangt af van een getal dat ergens anders staat. Als iemand later een extra `push_back` toevoegt, komt de fout terug zonder dat iets verandert op de plek van de referentie. Clang en cppcheck melden de code toch. |
| De referentie pas na de wijziging maken | Correct en het eenvoudigst. Kies deze oplossing als het kan. |

## Herkennen in een code-review

| Signaal | Waarom verdacht |
| --- | --- |
| `T&`, `T*` of een iterator naar een element, en daarna `push_back`, `emplace_back`, `insert`, `resize` of `reserve` op dezelfde vector | Elke functie die de capaciteit kan vergroten, maakt alle referenties ongeldig |
| `push_back` binnen `for (auto x : v)` over dezelfde `v` | De range-for bewaart `begin()` en `end()` van het oude blok |
| Een functie die een `std::vector<T>&` krijgt en tegelijk een `T&` naar een element ervan | De aanroeper kan een element van dezelfde vector doorgeven |
| Een `reserve()` die de juistheid van verderop liggende code moet garanderen | Correct tot iemand de aantallen verandert |

## Veilig alternatief

| Situatie | Alternatief |
| --- | --- |
| Een element onthouden terwijl de vector groeit | Een index (`std::size_t`) |
| Toevoegen tijdens het doorlopen | Een index-lus tot de oorspronkelijke `size()` (`append_evens`) |
| De waarde is genoeg | Kopieer de waarde vóór `push_back` |
| Elementen moeten een vast adres houden | Een andere container: `std::deque` (bij toevoegen aan de uiteinden) of `std::list`, of een vector van `std::unique_ptr<T>` |
| Altijd | Draai de tests ook onder ASan, en in debug én release |

## Bronnen

- cppreference: [`std::vector::push_back`](https://en.cppreference.com/w/cpp/container/vector/push_back),
  onderdeel "iterator invalidation".
- cppcheck: controles `invalidContainer`, `invalidContainerReference` en `invalidContainerLoop`.
- libstdc++: [debug mode](https://gcc.gnu.org/onlinedocs/libstdc++/manual/debug_mode.html).
