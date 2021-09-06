# JSON_Summator
Handler of JSON for Apache Kafka

В репозитории две ветки: основная (с менеджером) и ветка без менеджера. 

Ветка без менеджера возможна из-за выбранной реализации кольцевого буфера, потоки воркеров берут сообщения прямо из него.

Добавление менеджера позволяет решить такую проблему: в ситуации, когда много воркеров одновременно ждут данные из буфера, сообщения уходят по одному, 
то есть пока один воркер забирает своё сообщение, другие ждут. Предлагается каждому воркеру создать свой буфер и пополнять его из менеджера, как только 
появляется место.

Краткое описание реализации: функция main в файле JSON_Summator.cpp валидирует параметры, переданные через командную строку, создаёт и запускает потоки 
менеджера и воркеров, а далее выступает в качестве consumer, то есть принимает сообщения из топика input Apache Kafka (используя библиотеку cppkafka https://github.com/mfontanini/cppkafka) и складывает их в кольцевой буфер (модифицированный вариант boost::circular_buffer, boundedbuffer.hpp). У каждого потока воркера есть свой буфер, который пополняется потоком-менеджером, если в буфере есть место и если в буферах предыдущих потоков места нет. Воркер принимает данные из своего буфера, десериализует их, проводит валидацию, в случае успеха основную работу, сериализацию результата и отправляет его в потокобезопасный асинхронный объект класса Producer cppkafka, который отправляет сообщения в топик output Apache Kafka. Валидация проводится в потоке-воркере по двум причинам: таким образом 
это действие выполняется параллельно, а также достигается универсальность, так как за специфику задачи отвечает только файл worker.cpp со своим заголовком, всё остальное приложение не зависит от типа данных и выполняемых преобразований.


