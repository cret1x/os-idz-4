# ИДЗ №3
## Амирханов Никита БПИ219
## Вариант 1
Задача о парикмахере. В тихом городке есть парикмахерская. Салон парикмахерской мал, работать в нем нем может только один парикмахер, обслуживающий одного посетителя. Есть несколько кресел для ожидания в очереди. Парикмахер всю жизнь обслуживает посетителей. Когда в салоне никого нет, он спит в кресле. Когда посетитель приходит и видит спящего парикмахера, он будет его, садится в кресло, «засыпая» на тот момент, пока парикмахер обслуживает его. Если посетитель приходит, а парикмахер занят, то он встает в очередь и «засыпает». После стрижки парикмахер сам провожает посетителя. Если есть ожидающие посетители, то парикмахер будит одного из них и ждет пока тот сядет в кресло парикмахера и начинает стрижку. Если никого нет, он снова садится в свое кресло и засыпает до прихода посетителя. Создать приложение, моделирующее рабочий день парикмахерской.

### Запуск кода
В задании на оценку 4-5:
```sh
gcc client.c -o client
gcc cutter.c -o cutter
gcc server.c ../queue.c -o server -lrt -lpthread
./server <port for cutter> <port for clients>
./client <server ip> <server port> <number of clients>
./cutter <server ip> <server port>
```
В заданиях 6-10:
```sh
gcc client.c -o client
gcc listener.c -o listener
gcc server.c ../queue.c -o server -lrt -lpthread
./server <port for cutter> <port for clients> <port for listeners> <multicast addr> <multicast port>
./listener <server ip> <server port>
./client <server ip> <server port> <number of cleints>
./cutter <server ip> <server port>
```


### 4-5
Имеется приложение, создается 2 сокета, один для клиентов, второй для парикмахера. Приложение клиентов генерирует новых клиентов и отправляет на сервер. Сервер добавляет их в очередь. Сервер проверяет эту очередь и отправляет очередного клиента к парикмахеру.
Пример работы:
```sh
./server 8000 8001
[SYSTEM] Service 'Cutter' is running on 0.0.0.0:8000
[SYSTEM] Service 'Clients' is running on 0.0.0.0:8001
[SYSTEM] Cutter connected!
[SYSTEM] Client connected!
[Server] New client in the queue #1
[Server] Sent client #1 to cutter
[Server] New client in the queue #2
[Server] Sent client #2 to cutter
[Server] New client in the queue #3
[Server] Sent client #3 to cutter
[SYSTEM] Client disconected!
[Server] Client #1 finished
[Server] Client #2 finished
[Server] Client #3 finished
[SYSTEM] Cutter disconected!

```
```sh
./client 127.0.0.1 8001 3
Generating new client #1
Generating new client #2
Generating new client #3

```
```sh
Waiting for new client
Got new client #1
Got new client #2
Got new client #3

```
### 6-10
К прошлому приложению добавляется возможность подключения наблюдаделя к серверу. При этом я сразу сделал и возможность подключения множества наблюдателей с возможностью их отключения и переподключения, а так же возможность отключать-подключать и клиента и парикмахера. Для этого используется мультикаст по адресу `224.0.0.1` Кроме того при выключении сервера, будет отправлено сообщение всем подключенным. И они завершат работу. 
Пример работы:
![example](./cutter.png)
