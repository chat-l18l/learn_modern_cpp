# e004 — Lambda met referentie-capture die langer leeft dan de variabele

**Catalogus:** G03 · **Status:** onderzocht · **Datum:** 2026-11-22

## Vraag

1. Een lambda vangt een lokale variabele by reference en leeft langer dan die variabele. Welk
   vangnet vindt dat?
2. Waar staan de gegevens van een capture: op de stack, in de lambda, op de heap?
3. Hoeveel stack kost een lambda, en hoe houd je dat onder controle op een apparaat met 8 KiB
   stack?
4. Hoe schrijf je hetzelfde gedrag zo dat eigenaar, levensduur en grootte zichtbaar zijn?

## Achtergrond

### Een lambda is een struct

Een lambda-expressie maakt een object van een klasse zonder naam: het closure-object. Elke
capture is een lid van die klasse. De compiler schrijft voor deze lambda:

```cpp
int count = 0;
auto tick = [&count] { return ++count; };
```

ongeveer deze struct:

```cpp
struct Tick {
    int *count;                              // [&count]: the address, not the value
    int operator()() const { return ++*count; }
};
Tick tick{&count};
```

| Capture | Lid in het closure-object | Grootte op x86-64 | Waar staan de gegevens |
| --- | --- | --- | --- |
| `[x]` | een kopie van `x` | `sizeof(x)` | in het closure-object |
| `[&x]` | het adres van `x` | 8 bytes (op een 32-bit microcontroller 4) | `x` blijft waar `x` stond |
| `[=]` in een lidfunctie | `this`, als een lid wordt gebruikt | 8 bytes | het object blijft waar het stond |
| `[buf = std::move(buf)]` | het verplaatste object | `sizeof(buf)` | in het closure-object |

### Waar staat het closure-object

Het closure-object staat waar je het neerzet, net als elke andere waarde:

| Code | Plaats van het closure-object |
| --- | --- |
| `auto f = [...] { ... };` | in het stackframe van de huidige functie |
| `call(f)` met een parameter by value | een kopie in het stackframe van `call` |
| `std::function<int()> g = f;` | in het `std::function`-object als het klein genoeg is; anders op de heap |
| een lid van een struct | in die struct |

Een referentie-capture is dus veilig zolang het closure-object niet langer leeft dan de
variabele. Het gevaar ontstaat als de lambda **ontsnapt**: teruggegeven, in een
`std::function` bewaard, of geregistreerd als callback.

### Twintig jaar geleden

In C++03 bestonden er geen lambda's. Je schreef een functor: een struct met `operator()`. De
leden waren zichtbaar. Een lambda is precies zo'n struct, maar de compiler schrijft de struct en
je ziet de leden niet.

## Verwachting

1. GCC en Clang waarschuwen niet als een `[&]`-lambda ontsnapt via `std::function`.
2. GCC waarschuwt bij `[=]` die `this` vangt: dat is sinds C++20 verouderd.
3. clang-tidy meldt `[=]` met `this` (`cppcoreguidelines-misleading-capture-default-by-value`).
   clang-tidy meldt de ontsnappende `[&]` niet.
4. ASan vangt `[=]` met een object dat zijn scope uit is (`stack-use-after-scope`). De
   ontsnappende `[&]` vangt ASan alleen met `detect_stack_use_after_return=1`.
5. `sizeof` van `[&x]` is `sizeof(int *)`. `sizeof` van een lambda is gelijk aan `sizeof` van
   de uitgeschreven struct.
6. `std::function` gebruikt de heap als de captures groter zijn dan 16 bytes (libstdc++).
7. Een by-value capture van een buffer van 4 KiB maakt het stackframe minstens 4 KiB groter.
   `-Wframe-larger-than=1024` meldt dat tijdens het compileren.

## Opzet

| Bestand | Inhoud |
| --- | --- |
| `demo_unsafe.cpp` | Twee lambda's die ontsnappen: `escape-ref` en `escape-this` |
| `demo_stack.cpp` | Drie functies met een buffer van 4 KiB: capture by value, by reference, en een statische buffer. Geen ongedefinieerd gedrag. |
| `counter.hpp/.cpp` | Het transparante alternatief: struct `Counter`, functie `counter_tick`, tabel `Counter_table` |
| `counter_test.cpp` | Unit tests van het alternatief, en metingen van `sizeof` van closures |
| `function_alloc_test.cpp` | Meting: wanneer gebruikt `std::function` de heap |

| Variant | Code | Wat er misgaat |
| --- | --- | --- |
| `escape-ref` | `return [&count] { return ++count; };` met `count` lokaal | Het stackframe van de functie bestaat niet meer |
| `escape-this` | `return [=] { return value; };` in een lidfunctie, object in een binnenste scope | `[=]` kopieert `this`, niet `value`. Het object bestaat niet meer. |

## Uitvoeren

```bash
pixi run check
```

Het stackgebruik per functie staat na het bouwen in een `.su`-bestand:

```bash
cat build/gcc-debug/experimenten/e004-lambda-capture/CMakeFiles/e004_demo_stack.dir/demo_stack.cpp.su
```

## Resultaat

Gemeten op 2026-11-22 met GCC 15.3.0, Clang 23.1.6, clang-tidy 23.1.6, cppcheck 2.22 en
libstdc++ 15, op x86-64.

### Vangnetten

| Vangnet | `escape-ref` | `escape-this` |
| --- | --- | --- |
| GCC 15 | niets | `-Wdeprecated`: implicit capture of 'this' via '[=]' |
| Clang 23 | `-Wlifetime-safety-return-stack-addr` | `-Wdeprecated-this-capture` |
| clang-tidy | `clang-analyzer-core.StackAddressEscape` | `cppcoreguidelines-misleading-capture-default-by-value` |
| cppcheck 2.22 | `returnDanglingLifetime` | `invalidLifetime` |
| ASan, GCC en Clang, standaardopties | `stack-use-after-return` | `stack-use-after-scope` |

Zonder sanitizer:

| Preset | `escape-ref` | `escape-this` |
| --- | --- | --- |
| `gcc-debug` | **`1 2`, exit 0** | **`42`, exit 0** |
| `gcc-release` | willekeurige getallen, exit 1 | **`42`, exit 0** |
| `clang-debug` | willekeurige getallen, exit 1 | **`42`, exit 0** |

### Grootte van closures

De unit tests in `counter_test.cpp` en `function_alloc_test.cpp` leggen deze metingen vast. Ze
slagen onder elke preset (de heap-meting alleen zonder sanitizer).

| Object | `sizeof` |
| --- | --- |
| `[&count]` | 8: één pointer |
| `[values]` met `std::array<int, 16>` | 64: de kopie |
| `[&count, step]` | gelijk aan `struct { int *count; int step; }` |
| `[]` | 1 |
| `std::function<int()>` zelf | 32 |

| Captures in een `std::function` | Heap-allocaties |
| --- | --- |
| 8 of 16 bytes | 0 |
| 17 of 4096 bytes | 1 |
| `[&big]` met `big` van 4096 bytes | 0 |

### Stackgebruik

Uit de `.su`-bestanden van `demo_stack.cpp`, in bytes:

| Functie | `gcc-debug` | `clang-debug` | `gcc-asan` | `gcc-release` |
| --- | --- | --- | --- | --- |
| `read_first_by_value` (`[buffer]`) | 8208 | 8200 | 8528 | ingevoegd in `main` |
| `read_first_by_reference` (`[&buffer]`) | 4128 | 4120 | 4336 | ingevoegd in `main` |
| `read_first_static` (`static` buffer) | 16 | 8 | 16 | ingevoegd in `main` |
| `main` | 32 | 24 | 32 | 16 |

`-Wframe-larger-than=1024` meldde `read_first_by_value` en `read_first_by_reference` tijdens het
compileren, met GCC en met Clang. Onder `clang-asan` staat in het `.su`-bestand `dynamic`: ASan
verplaatst lokale variabelen naar een eigen stack om `stack-use-after-return` te kunnen vinden.

## Conclusie

1. Verwachting 1 klopt voor GCC, niet voor Clang. Clang 23 meldt de ontsnappende `[&count]`.
2. Verwachting 2 klopt. Beide compilers melden `[=]` met `this` als verouderd.
3. Verwachting 3 klopt maar half. clang-tidy meldt `[=]` met `this`, en meldt via de clang
   static analyzer ook de ontsnappende `[&count]`. cppcheck meldt beide.
4. Verwachting 4 klopt niet helemaal. ASan vangt beide varianten, ook zonder
   `detect_stack_use_after_return=1`: GCC 15 en Clang 23 zetten die controle standaard aan.
5. Verwachting 5 klopt. Een lambda is een struct. `[&x]` kost één pointer.
6. Verwachting 6 klopt. Boven 16 bytes aan captures alloceert `std::function` op de heap, zonder
   dat de code dat laat zien.
7. Verwachting 7 klopt. `[buffer]` verdubbelt het stackframe. `-Wframe-larger-than` meldt het
   tijdens het compileren.
8. Belangrijkste les: in `gcc-debug` geven beide fouten het **juiste** antwoord. `escape-this`
   geeft overal het juiste antwoord. Een test zonder sanitizer is groen.
9. Anders dan bij e001 vinden de statische hulpmiddelen deze fout goed. Het patroon "adres van
   een lokale variabele verlaat de functie" staat binnen één functie.

## Antwoorden op de vragen

### Waar staan de gegevens van een capture?

In het closure-object. Een capture by value is een kopie in dat object. Een capture by reference
is een pointer in dat object; de variabele zelf blijft waar ze stond. Het closure-object staat
waar je het neerzet: in het stackframe, in een struct, of in een `std::function`. Een
`std::function` zet captures boven 16 bytes op de heap.

### Hoe houd je het stackgebruik onder controle met 8 KiB stack?

| Maatregel | Wat het vangt | Wanneer |
| --- | --- | --- |
| `-Wframe-larger-than=<budget>` | Eén functie met een te groot stackframe | Tijdens het compileren |
| `-fstack-usage` | Het stackframe van elke functie, in een `.su`-bestand | Tijdens het compileren |
| `static_assert(sizeof(T) <= N)` op elke struct met toestand | Een struct die groeit door een nieuw lid | Tijdens het compileren |
| Grote buffers in `static` opslag of in de tabel die de eigenaar is | Grote objecten op de stack | Ontwerp |
| Geen `std::function`; de toestand direct in een struct met vaste grootte | Verborgen heap-allocaties | Ontwerp |

Dit experiment leidde tot besluit B16: de veilige code bouwt nu met
`-Wframe-larger-than=512`, en tot besluit B17: de aanroeper alloceert.

`-Wframe-larger-than` controleert één functie, niet een hele keten van aanroepen. Het totale
stackgebruik is de som langs de diepste keten. GCC kan die keten tonen met
`-fcallgraph-info=su`. Dat is in dit experiment niet gemeten.

Meet het stackgebruik met de optimalisatie van het apparaat. Onder `-O2` verdwenen alle buffers
uit dit experiment, omdat de waarde vooraf te berekenen was. Onder ASan zijn de getallen niet
bruikbaar.

### Hoe maak je het transparanter?

Schrijf de struct uit die de compiler anders voor je schrijft, en laat hem geen pointers
bevatten:

```cpp
// Before: what is stored, who owns count, and where does the closure live?
int count = 0;
scheduler.add([&count] { ++count; });

// After: the state is named, owned by the table, and has a fixed size.
struct Counter { int count = 0; int step = 1; };
int counter_tick(Counter &counter);

struct Counter_table {
    std::array<Counter, max_counters> counters{};
    std::size_t used = 0;
};
static_assert(sizeof(Counter_table) <= 48);

static Counter_table s_counters;               // static storage: no stack, no heap
counter_table_add(s_counters, {.count = 0, .step = 1});
counter_table_tick_all(s_counters);
```

| Vraag | Lambda met `[&]` | Struct en tabel |
| --- | --- | --- |
| Wat wordt bewaard? | Verborgen: een pointer | Zichtbaar: de leden van `Counter` |
| Wie is de eigenaar? | De functie waar `count` staat | `Counter_table` |
| Hoe lang leeft de toestand? | Tot die functie terugkeert | Zolang de tabel bestaat |
| Hoe groot? | Onbekend zonder meting | `static_assert` |
| Heap? | Mogelijk, via `std::function` | Nooit |

## Herkennen in een code-review

| Signaal | Waarom verdacht |
| --- | --- |
| `[&]` of `[&x]` in een lambda die wordt teruggegeven, in een `std::function` gaat, of als callback wordt geregistreerd | De lambda kan langer leven dan `x` |
| `[=]` in een lidfunctie | Kopieert `this`, niet de leden |
| `[this]` in een callback | Leeft het object zolang de callback? |
| Een capture by value van een array of grote struct | Kopie op de stack, of op de heap via `std::function` |
| `std::function` op een apparaat zonder heap | Alloceert boven 16 bytes aan captures |

## Veilig alternatief

| Situatie | Alternatief |
| --- | --- |
| De lambda draait direct, binnen de scope (`std::sort`, `std::ranges::for_each`) | `[&]` is veilig en duidelijk |
| De lambda ontsnapt, kleine toestand | Capture by value, met namen: `[count = count]` |
| De lambda ontsnapt in een lidfunctie | Vang de leden die je nodig hebt by value, niet `this` |
| Toestand die langer leeft dan één aanroep | Een struct met de toestand, een vrije functie, en een tabel die de eigenaar is |
| Embedded, geen heap | Geen `std::function`. Een tabel met vaste grootte, in `static` opslag, met `static_assert` op de grootte. |

## Bronnen

- cppreference: [lambda expressions](https://en.cppreference.com/w/cpp/language/lambda),
  onderdeel "Lambda capture".
- GCC: [`-fstack-usage`, `-Wframe-larger-than`, `-fcallgraph-info`](https://gcc.gnu.org/onlinedocs/gcc/Developer-Options.html).
- AddressSanitizer: [stack-use-after-return](https://github.com/google/sanitizers/wiki/AddressSanitizerUseAfterReturn).
