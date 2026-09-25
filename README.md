# Emergency Handler

Progetto di Laboratorio II: sistema multithread in **C11** per la gestione di emergenze all'interno di una griglia bidimensionale.

Il sistema riceve richieste tramite una coda di messaggi POSIX e coordina l'assegnazione dei soccorritori in base a priorità, disponibilità e distanza. Ogni soccorritore è rappresentato da un gemello digitale (*digital twin*), che ne descrive tipo, posizione e stato operativo. Gli eventi significativi sono registrati nei log.

## Requisiti

**Il progetto richiede un ambiente Linux per essere compilato ed eseguito.** Il sistema di riferimento indicato nella relazione è **Ubuntu 24.04**.

Sono necessari:

- GCC con supporto allo standard C11 e ai thread C11 (`<threads.h>`).
- GNU Make.
- Supporto alle code di messaggi POSIX (`<mqueue.h>`).

Se non si dispone di un ambiente Linux, utilizzare all'occorrenza:

- **Una macchina virtuale (VM) con Linux**, ad esempio Ubuntu 24.04, su Windows o macOS.
- **WSL (Windows Subsystem for Linux) con Ubuntu**, se si utilizza Windows.

La compilazione e l'esecuzione di server e client devono avvenire all'interno dello stesso ambiente Linux scelto, usando il terminale della VM o di WSL.

## Architettura

### Client

Invia emergenze singole dalla riga di comando oppure più richieste da un file. Ogni richiesta viene inviata da un thread dedicato dopo il ritardo specificato, espresso in secondi. Il client attende la conclusione dei thread prima di terminare.

### Server

All'avvio legge i file di configurazione e inizializza le strutture dati del sistema. La gestione concorrente si articola in:

- **Receiver**: riceve i messaggi dalla coda POSIX, controlla nome dell'emergenza, coordinate e timestamp, quindi inserisce le richieste valide nella coda interna a priorità.
- **Dispatcher**: estrae le emergenze dalla coda e avvia un thread dedicato per ciascuna richiesta.
- **Gestore dell'emergenza**: verifica le risorse disponibili e i tempi di arrivo, coordina i soccorritori e aggiorna lo stato dell'intervento.
- **Digital twin**: rappresenta il soccorritore e ne simula gli spostamenti verso l'emergenza e il ritorno alla base.

La coda prioritaria è realizzata tramite una lista collegata ordinata. Mutex e variabili di condizione coordinano l'accesso alle strutture condivise e l'attesa delle risorse.

La relazione descrive lo stato `TIMEOUT` per le emergenze che non possono essere gestite entro il tempo previsto e `CANCELED` quando le risorse complessive del sistema sono insufficienti.

### Parser e logger

I parser leggono i file `.conf` e costruiscono le strutture dati in memoria. Il logger registra ricezione delle richieste, assegnazioni, cambiamenti di stato, completamenti e timeout, associando agli eventi timestamp e identificativi.

## Struttura del progetto

```text
.
├── conf/                      # Configurazione dell'ambiente e delle risorse
│   ├── env.conf
│   ├── emergency_types.conf
│   └── rescues.conf
├── include/                   # Header, strutture dati e macro condivise
├── logs/                      # Log di esecuzione
├── src/
│   ├── client/                # Invio delle richieste
│   ├── server/                # Server, thread e gestione delle emergenze
│   ├── parsing/               # Parser dei file di configurazione
│   ├── logging/               # Logger
│   ├── utils/                 # Coda prioritaria, digital twin e supporto
│   └── other/                 # Funzioni ausiliarie
├── emergenze_da_inviare.txt    # File di richieste presente nel progetto
└── Makefile
```

## Configurazione

Prima dell'avvio, controllare i file nella directory `conf/`:

| File | Contenuto |
| --- | --- |
| `env.conf` | Nome della coda POSIX e dimensioni della griglia. |
| `rescues.conf` | Tipi di soccorritori, quantità, velocità e coordinate delle basi. |
| `emergency_types.conf` | Tipi di emergenza, priorità e risorse richieste. |

I nomi delle emergenze inviati dal client devono corrispondere a quelli definiti nella configurazione e le coordinate devono rientrare nella griglia.

## Compilazione

Eseguire i comandi dalla directory principale del progetto, poiché i percorsi dei file di configurazione sono relativi a essa.

Per compilare server e client:

```bash
make all
```

Per compilarli separatamente:

```bash
make server
make client
```

Gli eseguibili generati sono `serverfile` e `clientfile`.

## Esecuzione

### 1. Avviare il server

```bash
make server_run
```

In alternativa, dopo la compilazione:

```bash
./serverfile
```

### 2. Inviare le emergenze

Aprire un secondo terminale nella directory principale del progetto, lasciando il server in esecuzione.

**Emergenza singola**

```bash
./clientfile <nome_emergenza> <coord_x> <coord_y> <delay_in_secs>
```

Esempio di invio immediato:

```bash
./clientfile Incendio 100 150 0
```

Il ritardo indica quanti secondi attendere prima di inviare la richiesta.

**Emergenze da file**

```bash
./clientfile -f emergenze_da_inviare.txt
```

Ogni riga deve contenere nome dell'emergenza, coordinate e ritardo, separati da spazi:

```text
Incendio 100 150 0
Allagamento 50 80 3
Sommossa 120 180 5
```

I ritardi delle richieste sono gestiti dai rispettivi thread e non sono attese cumulative tra una riga e la successiva. Il client gestisce al massimo 100 richieste per esecuzione e ignora le righe che iniziano con `#`.

### 3. Arrestare il server

Premere **Ctrl+C** nel terminale del server. Il segnale `SIGINT` attiva la procedura di terminazione.

## Log

Consultare la directory `logs/` per seguire l'esecuzione e controllare gli eventi registrati dal sistema.

## Pulizia

Per rimuovere gli eseguibili e gli eventuali file oggetto:

```bash
make clean
```

## Documentazione di riferimento

Questo README è basato sulla relazione [PROGETTO LAB II (1).pdf](<PROGETTO LAB II (1).pdf>). I percorsi dei moduli e la sintassi dei comandi sono stati adattati alla struttura e al codice presenti nel repository.
