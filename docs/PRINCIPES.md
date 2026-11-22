# Ontwerpprincipes

Deze principes beschrijven betrouwbare, eenvoudige software: minimale abstracties, geen diepe
klassenhiërarchieën, en data en control flow die je kunt volgen. Elk principe heeft een regel,
een reden, een toets en bewijs.

De principes nemen de ideeën van data-oriented design (DOD) over die de data flow zichtbaar
maken: data eerst, tabellen, getypeerde indexen, transformaties met een eigen naam. De
hardware-optimalisaties uit DOD vallen onder P7. De afweging per onderwerp staat in besluit B19.

De principes gelden voor alle nieuwe code in dit project. Ze zijn ook bedoeld om code en
architectuur van andere projecten te toetsen, en code van anderen. De vragen voor een audit staan
in [AUDITVRAGEN.md](AUDITVRAGEN.md).

## Overzicht

| ID | Principe | Kern |
| --- | --- | --- |
| P1 | Data en control flow zijn gescheiden | Data in structs, transformaties in functies, beslissingen op één plek |
| P2 | Elke data heeft één eigenaar | Eén plek beheert het geheugen en schrijft. De rest leent, korter dan de eigenaar leeft. |
| P3 | Control flow is expliciet | Toestanden in een `enum class`, overgangen op één plek, geen verborgen sprongen |
| P4 | Minimale abstractie | Concrete code. Elke abstractie heeft een reden die je kunt aanwijzen. |
| P5 | Geheugengebruik is vast en zichtbaar | De aanroeper alloceert. Een stackbudget per functie. |
| P6 | Code van anderen staat achter één grens | Eerst controleren, dan gebruiken, en alleen via een wrapper |
| P7 | Helderheid gaat voor performance | Een optimalisatie alleen na een eis die faalt, in overleg, met de heldere versie als referentie |

| Status | Betekenis |
| --- | --- |
| vastgesteld | Het principe heeft een toets en minstens één verwijzing naar bewijs |
| werkhypothese | Het principe heeft nog geen bewijs uit een experiment |

| ID | Status | Bewijs |
| --- | --- | --- |
| P1 | vastgesteld | e004 |
| P2 | vastgesteld | e001, e002, e004, e005 |
| P3 | werkhypothese | G27: voorlopige meting, nog geen experiment |
| P4 | werkhypothese | – |
| P5 | vastgesteld | e004, B16 |
| P6 | vastgesteld | e003 |
| P7 | werkhypothese | – |

---

## P1 — Data en control flow zijn gescheiden

### Regel

1. Data staat in eenvoudige structs, zonder gedrag.
2. Een transformatie is een vrije functie: data in, data uit. Een transformatie is bij voorkeur
   een pure functie.
3. De control flow van een taak (wat er wanneer gebeurt) staat op één plek: een state machine of
   een tabel. De control flow zit niet verspreid over callbacks, virtuele functies of namen van
   functies.
4. Invoer en uitvoer (hardware, bestanden, netwerk, tijd) staan in weinig functies met een
   duidelijke naam. Het rekenwerk staat ertussen, in pure functies.
5. De kern is een pure functie per element: `Sample filter_one(Sample)`. Een batchfunctie over
   een `std::span` is een dunne lus om die functie, en bestaat alleen als er een batch is.
6. Een transformatie is van één van drie soorten. Elke soort heeft een eigen vorm:

| Soort | Vorm | Regel |
| --- | --- | --- |
| A→B | `f(std::span<const A> in, std::span<B> out)` | `in` en `out` overlappen niet. De functie assert dat. |
| In place | `f_in_place(std::span<A> data)` | `_in_place` in de naam |
| Generatief | `f(std::span<const A> in, std::span<B> out)` plus `constexpr std::size_t f_max_output(std::size_t n)` | De aanroeper maakt `out` tijdens het compileren groot genoeg met `f_max_output`. Gaat het toch mis, dan geeft `f` een fout terug, en is de inhoud van `out` ongedefinieerd. Het contract zegt dat. |

Een A→B-functie en een generatieve functie geven terug hoeveel elementen zij schreven, of een
fout via `std::expected`.

### Waarom

Een fout zoeken is twee vragen beantwoorden. Welke data was fout? Welke beslissing leidde
daartoe? Als data en control flow door elkaar lopen, moet je bij elke fout reconstrueren hoe het
geheel is samengesteld. Als ze gescheiden zijn, beantwoord je de twee vragen apart. Een audit en
een uitleg worden daardoor korter.

Een pure functie is eenvoudig te testen. Dezelfde invoer geeft altijd dezelfde uitkomst, en de
test hoeft geen toestand op te bouwen.

### Toets

| Toets | Soort |
| --- | --- |
| A1–A5, A9, A20 | auditvraag |
| Een variabele of parameter die `const` kan zijn | clang-tidy `misc-const-correctness` |

### Bewijs

| Bron | Wat het laat zien |
| --- | --- |
| [e004](../experimenten/e004-lambda-capture/README.md) | Een callback met `[&count]` verbergt data (een pointer), eigenaar en levensduur in de control flow. Uitgeschreven als struct `Counter`, functie `counter_tick` en tabel `Counter_table` zijn alle drie zichtbaar, en kan niets meer hangen. |

### Achtergrond

| Bron | Gedachte |
| --- | --- |
| Fred Brooks, *The Mythical Man-Month* (1975) | "Show me your tables, and I won't usually need your flowcharts." |
| Rob Pike, *Notes on Programming in C* (1989), regel 5 | "Data dominates." |
| Gary Bernhardt, *Boundaries* (2012) | "Functional core, imperative shell": pure kern, dunne schil met invoer en uitvoer |
| Mike Acton, *Data-Oriented Design and C++* (CppCon 2014) | Eerst de data, dan de code |
| Melvin Conway, *How Do Committees Invent?* (1968) | Een systeem neemt de structuur over van de communicatie in de organisatie die het bouwt |

---

## P2 — Elke data heeft één eigenaar

### Regel

1. Elk object met data heeft één eigenaar. De eigenaar beheert het geheugen en is de enige die
   de data mag veranderen.
2. Een functie die de data alleen leest, krijgt `const T &` of `std::span<const T>`.
3. Een functie die de data mag veranderen, krijgt `T &` of `std::span<T>`, voor de duur van
   één aanroep.
4. Een view, referentie, pointer, iterator of referentie-capture leeft niet langer dan de
   aanroep. Een uitzondering heeft commentaar dat de eigenaar en de levensduur noemt.
5. De invarianten van de data staan bij de data. Twee manieren, bij voorkeur de eerste:
   - types maken een ongeldige toestand onmogelijk: `enum class`, `std::variant`,
     `std::array` met een vaste grootte;
   - de transformatiefunctie controleert de invariant: `assert` bij een programmeerfout,
     `std::expected` bij invoer van buiten.

6. Elke tabel heeft een vaste grootte, en er wordt niets uit verwijderd. Elke tabel heeft een
   eigen handle-type: een sterk type, met de hand uitgeschreven, één struct per tabel. Een kale
   `std::size_t` als index is niet toegestaan.

```cpp
struct Sensor_id {
    explicit constexpr Sensor_id(std::uint16_t v) : value{v} {}
    std::uint16_t value;
    friend constexpr bool operator==(Sensor_id, Sensor_id) = default;
};
```

| Regel voor een handle | Waarom |
| --- | --- |
| Een `explicit` constructor | Zonder die constructor wordt een losse `{3}` stil een handle: `sensor_at(table, {3})` compileert dan |
| Alleen `==` | Een handle is een naam, geen getal. Optellen, ordenen en omzetten naar een getal hebben geen betekenis. |
| `.value` lezen alleen in de functies van de tabel zelf, en aan de rand (opslaan, versturen) | Het getal blijft binnen de tabel |
| Een handle uit een getal maken alleen in de tabel (`<x>_add`), in `<x>_find` (een ID van buiten) en in tests | Elke andere `Sensor_id{...}` is een bevinding in de review. Zoek met `grep 'Sensor_id{'`. |
| Eén uitgeschreven struct per tabel, geen template | De struct is in één oogopslag te lezen, en de foutmeldingen blijven kort |

Gemeten op 2026-11-22 met GCC 15.3 en Clang 23.1, met `static_assert` tijdens het compileren.
De tests staan in [e005](../experimenten/e005-sterke-types/README.md):

| Fout | Kale `std::size_t` | `struct { std::uint16_t value; }` | Sterk type hierboven |
| --- | --- | --- | --- |
| Een handle van een andere tabel | compileert | geweigerd | geweigerd |
| Een `int` als handle | compileert | geweigerd | geweigerd |
| `sensor_at(table, {3})` | compileert | **compileert** | geweigerd |
| `id + 1`, `id < id` | compileert | geweigerd | geweigerd |
| `id == id` | compileert | **geweigerd: geen `==`** | compileert |
| `Sensor_id == Actuator_id` | compileert | geweigerd | geweigerd |
| Grootte | 8 bytes | 2 bytes | 2 bytes, trivially copyable |

De kolom "kale `std::size_t`" volgt uit de taalregels en is niet apart gemeten.

7. Een tabel heeft twee toegangen met twee namen:

| Functie | Voor | Bij een ongeldige index |
| --- | --- | --- |
| `sensor_at(table, id)` | een index uit eigen code | `assert`: een programmeerfout |
| `sensor_find(table, id)` | een ID van buiten, bijvoorbeeld uit een netwerkbericht | `std::expected` met een fout |

8. Een tabel waaruit wél wordt verwijderd, gebruikt een handle met generatie: index plus een
   teller die ophoogt als een slot opnieuw wordt gebruikt. Een verouderde gewone index leest het
   verkeerde element, en ASan ziet dat niet: het geheugen is geldig. Zie catalogus G28.
9. Optionele gegevens staan in de struct zelf, als `std::optional`. Een aparte tabel
   (side table) mag alleen als het geheugen een gemeten eis is (P7). Het contract van de side
   table noemt dan de invariant: elke ID in de side table bestaat in de hoofdtabel.

10. Een fysieke grootheid aan een interface tussen modules (een functie in een header) is een
    sterk type, met de eenheid en de resolutie in de naam: `Millivolt`, `Deci_celsius`. De vorm is
    die van een handle: een `explicit` constructor en één lid `value`. Het type krijgt alleen de
    bewerkingen die de grootheid nodig heeft, bijvoorbeeld `Millivolt + Millivolt`. Tijd is een
    `std::chrono`-type: `std::chrono::milliseconds`. Binnen een pure rekenfunctie mag een kaal
    getal. De omzetting staat dan aan het begin en het einde van de functie, via `.value`.
11. Een sterk type ziet geen verwisseling van twee waarden van hetzelfde type:
    `alarm(Millivolt low, Millivolt high, ...)` aangeroepen als `alarm(high, low, ...)`
    compileert. Twee of meer waarden van hetzelfde type gaan daarom in een struct met benoemde
    velden, en de aanroeper gebruikt designated initializers:
    `Limits{.low = Millivolt{100}, .high = Millivolt{900}}`. De verkeerde volgorde is een fout
    in GCC, en in Clang alleen met `-Werror` (B12). Een veld zonder default constructor kan de
    aanroeper niet overslaan.

Regel 5 is nodig omdat P1 de data losmaakt van het gedrag. Object-oriëntatie beschermt
invarianten door data in een klasse te verbergen. Zonder die klasse moet de bescherming uit de
types en uit de transformaties komen.

### Waarom

Alle constructies in fase 1 van de [catalogus](CATALOGUS.md) zijn fouten in eigenaarschap:
iets leent van een eigenaar die al weg is. Een fout in eigenaarschap is moeilijk te testen. Het
vrijgegeven geheugen bevat vaak nog de oude waarde, en de test is groen.

### Toets

| Toets | Soort |
| --- | --- |
| A1–A5, A17, A18, A21, A22 | auditvraag |
| `static_assert` met een `concept` op de eigenschappen van een handle | compiler |
| `bugprone-easily-swappable-parameters`: twee naast elkaar staande parameters van hetzelfde type | clang-tidy |
| `readability-suspicious-call-argument`: een argument waarvan de naam bij een andere parameter past | clang-tidy |
| Een compile-fail-test (`leren_compile_fail_test`) | ctest |
| `[[clang::lifetimebound]]` op een parameter waarvan het resultaat leent | Clang |
| `-Wlifetime-safety` | Clang 23 |
| `invalidContainer`, `returnDanglingLifetime`, `invalidLifetime` | cppcheck |
| `heap-use-after-free`, `stack-use-after-scope`, `stack-use-after-return` | ASan |

### Bewijs

| Bron | Wat het laat zien |
| --- | --- |
| [e001](../experimenten/e001-dangling-string-view/README.md) | Een `std::string_view` naar een tijdelijke `std::string`. Alleen ASan vindt het altijd. Zonder sanitizer geeft een korte string het juiste antwoord. |
| [e002](../experimenten/e002-vector-push-back/README.md) | Een referentie in een vector na `push_back`. Of de code fout is, hangt af van de capaciteit: een getal dat niet in de code staat. |
| [e004](../experimenten/e004-lambda-capture/README.md) | Een lambda die ontsnapt. In `gcc-debug` geeft de fout het juiste antwoord. |
| [e005](../experimenten/e005-sterke-types/README.md) | Een sterk handle-type weigert een handle van een andere tabel, een `int` en `{3}`, vastgelegd met `static_assert`. Een verouderde gewone index leest een verkeerd element zonder dat een sanitizer het ziet. Een generatie van 16 bits loopt na 65536 keer hergebruik rond. |

---

## P3 — Control flow is expliciet

### Regel

1. De toestanden van een taak staan in één `enum class`.
2. Eén functie berekent de volgende toestand, met een `switch` over de huidige toestand zonder
   `default`. Er is één punt waar de nieuwe toestand wordt vastgelegd.
3. Een keuze tussen gevallen is een tabel, geen keten van `if`/`else`.
4. De toestand van een element is een `enum class`-veld in het element, niet de tabel waarin
   het staat. Elementen naar toestand over tabellen verdelen (partitioneren op toestand) mag
   alleen onder P7.
5. Geen control flow die je niet ziet op de plek van de aanroep:
   - geen overloads die alleen in parametertype verschillen;
   - geen impliciete conversies naar `bool` of tussen getaltypes;
   - geen virtuele aanroep of callback als een `switch` hetzelfde doet.

### Waarom

Een tabel of een `switch` laat in één oogopslag zien of elk geval is afgedekt. Een keten van
`if`'s doet dat niet. Bij een fout zie je de toestand en de gebeurtenis, en de tabel zegt wat er
had moeten gebeuren.

### Toets

| Toets | Soort |
| --- | --- |
| A6–A8 | auditvraag |
| `-Wswitch` (in `-Wall`), `-Wimplicit-fallthrough` | compiler |
| `-Wconversion`, `-Wsign-conversion`, `-Wstring-conversion` | compiler |
| `readability-implicit-bool-conversion` | clang-tidy |

### Bewijs

| Bron | Wat het laat zien |
| --- | --- |
| [G27](CATALOGUS.md) (voorlopige meting) | `log_it("hello")` roept `log_it(bool)` aan, niet `log_it(const std::string &)`. GCC 15 meldt niets. |

---

## P4 — Minimale abstractie

### Regel

| Regel | Uitzondering |
| --- | --- |
| Schrijf concrete functies en structs. Een template alleen als dezelfde logica voor minstens drie echt verschillende types nodig is. | – |
| Begrens elke template-parameter met een C++20 `concept` of een `requires`-clausule. | – |
| Geef elke functie een eigen naam: `parse_from_file`, `parse_from_string`. | Constructors, operators (`==`, `<=>`, `<<`), en namen die de standaardbibliotheek voorschrijft |
| Houd gedrag bij elkaar. Liever één duidelijke functie van 40 regels dan tien helpers verspreid over bestanden. Splits pas als het deel een eigen naam verdient of een eigen test nodig heeft. | – |
| `virtual` alleen voor runtime-polymorfie met een open verzameling types. Een gesloten verzameling is een `enum class` met een `switch`, of een `std::variant`. | – |
| Een klasse met `virtual` heeft een `virtual` of `protected` destructor. Geef een object van zo'n klasse nooit by value door. | – |
| Elke soort ding heeft een eigen tabel met een eigen index-type: één tabel voor sensoren, één voor actuatoren. Geen gemengde tabel met `std::variant` of `virtual` als de soorten vooraf bekend zijn. | – |
| Geen framework dat de control flow overneemt, zoals een Entity Component System. De ideeën wel: ID's, tabellen, functies over tabellen. De volgorde van de stappen in een taak staat expliciet in één functie of state machine. | – |
| Een lambda die ontsnapt (teruggegeven, in een `std::function`, als callback) vangt niets by reference en gebruikt geen `[=]` in een lidfunctie. | Een lambda die direct draait binnen de scope (`std::sort`, `std::ranges::for_each`) mag `[&]` gebruiken |
| Commentaar legt uit waarom, en wat het contract is: eigenaar, levensduur, voorwaarden. Geen commentaar dat herhaalt wat de code zegt. | In een demo mag commentaar het mechanisme uitleggen |

### Waarom

Elke abstractie verplaatst een deel van de betekenis naar een andere plek. Een lezer, een
auditor of een agent moet die plek dan vinden. Een abstractie is de moeite waard als zij meer
uitleg bespaart dan zij kost.

### Toets

| Toets | Soort |
| --- | --- |
| A10 | auditvraag |
| `readability-function-cognitive-complexity` | clang-tidy |
| `modernize-use-constraints` | clang-tidy |
| `-Wnon-virtual-dtor`; `cppcoreguidelines-virtual-class-destructor`, `cppcoreguidelines-slicing` | compiler, clang-tidy |
| `clang-analyzer-core.StackAddressEscape`, `cppcoreguidelines-misleading-capture-default-by-value` | clang-tidy |

### Bewijs

Nog geen experiment. Fase 6 van het [plan](PLAN.md) toetst dit principe aan code uit de
praktijk.

---

## P5 — Geheugengebruik is vast en zichtbaar

### Regel

Het ontwerpdoel is een stack van 4 KiB per taak. Op de stack staan parameters, referenties en
kleine lokale waarden. Grote gegevens staan er niet.

1. Een functie alloceert geen buffers. De aanroeper alloceert en geeft de buffer mee als
   `std::span<T>` (schrijven) of `std::span<const T>` (lezen).
2. Een functie geeft geen eigen container terug (`std::string`, `std::vector`). Het resultaat
   gaat in een buffer van de aanroeper. De functie geeft terug hoeveel zij schreef, of een fout
   via `std::expected`.
3. Grote gegevens staan in `static` opslag of in een struct die de eigenaar is.
4. Elke functie past in het stackbudget van 512 bytes (`LEREN_FRAME_LIMIT`).
5. Een struct met toestand krijgt een `static_assert` op `sizeof`.
6. Geen `std::function` in code voor een apparaat. Een `std::function` gebruikt de heap boven 16
   bytes aan captures, zonder dat de code dat laat zien.

| Uitzondering op het stackbudget | Waarom |
| --- | --- |
| Tests (`leren_gtest`, markering `HOST`) | Draaien alleen op de ontwikkelmachine |
| Demo's (`UNSAFE`) | Hun meldingen zijn meetgegevens |
| Presets met een sanitizer | ASan maakt stackframes groter. Een apparaat draait niet met ASan. |

### Waarom

Het geheugengebruik is dan vooraf bekend en staat in de code. Een stackoverflow op een apparaat
is een fout die je pas in het veld ziet. Een te groot stackframe dat niet compileert, zie je
aan je bureau.

### Toets

| Toets | Soort |
| --- | --- |
| A11–A13 | auditvraag |
| `-Wframe-larger-than=512` met `-Werror` | compiler, presets zonder sanitizer |
| `-fstack-usage` | compiler: stackframe per functie |
| `static_assert(sizeof(T) <= N)` | compiler |

### Bewijs

| Bron | Wat het laat zien |
| --- | --- |
| [e004](../experimenten/e004-lambda-capture/README.md) | `[buffer]` met 4 KiB verdubbelt het stackframe: 8208 tegen 4128 bytes. Een `static` buffer: 16 bytes. `std::function` alloceert boven 16 bytes aan captures. |
| [B16](BESLUITEN.md) | Een geplante buffer van 600 bytes stopt de bouw. De grootste functie in de bibliotheekcode gebruikt 128 bytes. |

De code in e001–e004 dateert van vóór deze regel. `e001::first_word_copy` geeft een
`std::string` terug en `e002::append_evens` laat de vector groeien. Die code blijft staan als
studiemateriaal.

---

## P6 — Code van anderen staat achter één grens

### Regel

1. Controleer code van anderen voordat zij in gebruik komt, en na elke update: `pixi run audit`.
2. Bouw code van anderen uit de bron mee in de presets met een sanitizer. Een bibliotheek die
   alleen als binair bestand bestaat, staat in het verslag als blinde vlek.
3. Roep code van anderen alleen aan via één wrapper. De wrapper gebruikt `std::span`,
   `std::in_range` en `std::optional` of `std::expected`. De wrapper controleert elke
   voorwaarde die de code van anderen aan de aanroeper overlaat.
4. Een functie van anderen die aantoonbaar fout is, gebruikt de wrapper niet.

### Waarom

De gewone bouw is stil over code van anderen. CMake voegt hun headers in als `-isystem`, en de
compiler meldt dan niets. ASan ziet niets in een bibliotheek die zonder ASan is gebouwd.

### Toets

| Toets | Soort |
| --- | --- |
| A14, A15 | auditvraag |
| `pixi run audit`: preset `clang-audit`, clang-tidy en cppcheck over `vreemd/` | hulpmiddel |
| ASan met de bibliotheek uit de bron | sanitizer |

### Bewijs

| Bron | Wat het laat zien |
| --- | --- |
| [e003](../experimenten/e003-code-van-anderen/README.md) | Vier geplante fouten: de gewone bouw meldt 0. De audit vindt er 2 precies en wijst de andere 2 aan. ASan mist de fout in de al gebouwde bibliotheek. |

---

## P7 — Helderheid gaat voor performance

### Regel

1. Eerst de heldere versie, volgens P1–P6.
2. Een herschrijving voor performance mag alleen als een **eis faalt**, en de meting dat laat
   zien. Voorbeeld: "de regellus moet binnen 1 ms klaar zijn; gemeten 1,4 ms op de ESP32-P4".
   Een profiel dat een hotspot aanwijst, zegt waar je moet kijken, niet dát het moet.
3. Een herschrijving voor performance gebeurt in overleg met de eigenaar van het project. Een
   besluit in `BESLUITEN.md` legt vast: de eis, de meting voor en na, en waarom de gekozen
   constructie nodig is.
4. De heldere versie blijft bestaan als referentie. De tests geven dezelfde invoer aan beide
   versies en vergelijken de uitkomst.
5. De data flow blijft te volgen, ook in de snelle versie. Het commentaar van de snelle versie
   verwijst naar de referentie.

Technieken uit DOD die alleen onder P7 vallen:

| Techniek | Wat het doet | Wat het kost aan helderheid |
| --- | --- | --- |
| Struct of Arrays | Elk veld in een eigen array, zodat een lus alleen de velden laadt die zij leest | Het begrip "één element" verdwijnt. De invariant "index `i` hoort in alle arrays bij hetzelfde element" staat nergens. |
| Hot/cold-splitsing | Velden die vaak worden gelezen, staan apart van velden die zelden worden gelezen | De gegevens van één element staan op twee plekken |
| Partitioneren op toestand | Elementen staan per toestand in een eigen tabel, zodat een lus geen `if` heeft | De toestand is niet meer zichtbaar als veld. Een overgang is verplaatsen. |
| Side table voor geheugen | Optionele gegevens in een kleine aparte tabel | Twee eigenaars van de gegevens van één element |

### Waarom

Code wordt vaker gelezen, geaudit en uitgelegd dan geoptimaliseerd. Een optimalisatie zonder
eis maakt de code moeilijker en lost geen probleem op. Een optimalisatie met een referentie kun
je toetsen aan een versie die je kunt uitleggen.

De doelplatforms zijn de ESP32-P4, de ESP32-S3 en Linux op x86-64. Alle drie hebben caches,
maar het effect verschilt per platform en per soort geheugen (intern RAM, PSRAM, flash). Een
optimalisatie die op de ene meting wint, kan op de andere verliezen. Meet op het doelplatform.

### Toets

| Toets | Soort |
| --- | --- |
| A19 | auditvraag |
| Een test die de snelle versie vergelijkt met de referentie | unit test |

### Bewijs

Nog geen experiment.
