Блокирующая очередь
====================

| Название | blocking-queue   |
| -------- | ---------------- |
| Баллы    | 4                |
| Дедлайн  | 24.10.2021 09:00 |


# Подготовка

Вспомните, как использовать [mutex](https://en.cppreference.com/w/cpp/thread/mutex) и [condition_variable](https://en.cppreference.com/w/cpp/thread/condition_variable).

Если вы не знаете, что такое std::optional, [узнайте](https://en.cppreference.com/w/cpp/utility/optional).

# Задание

Реализуйте блокирующую очередь с ограниченным буфером. Состояние очереди описывается элементами, которые в ней лежат, и флагом closed. Изначально очередь открыта (`closed=false`). Количество элементов ограничено числом `max_size`, которое передаётся в конструктор. Очередь поддерживает следующие операции:

* `bool put(T element)`

  Если количество элементов в очереди достигло максимума, метод блокируется, пока не освободиться свободное место или пока очередь не закроют.
  Если очередь закрыта, возвращает `false`. Иначе добавляет элемент в конец очереди и возвращает `true`.

* `bool offer(T element)`

  Вставляет элемент в конец очереди, если она не закрыта и в ней есть свободное место.
  Если получилось вставить, возвращает `true`, иначе `false`.

* `std::optional<T> take()`

  Если очередь пуста, метод блокируется, пока не появится элемент или пока очередь не закроют.
  Если в очереди есть элементы, извлекает элемент из начала очереди и возвращает его.
  Если очередь закрыта и элементов нет, возвращает пустой `optional`.

* `void close()`

  Закрывает очередь.

* `bool is_closed()`

  Проверяет, закрыта ли очередь.

# Реализация

Напишите свою реализацию в файле `blocking-queue/blocking_queue.hpp`. Там уже есть какой-то шаблон. Его можно менять, но название класса и публичные методы нужно сохранить.

Менять `blocking-queue/tests/main.cpp` нельзя. Если вдруг вы захотите добавить свои тесты, создайте отдельный файл и добавьте вызов `make_test` в `blocking-queue/CMakeLists.txt`.


# Вопросы для размышления

1. Рассмотрим следующую реализацию пула потоков, использующую вашу очередь.
    ```cpp
    class ThreadPool {
    public:
        ThreadPool(size_t num_threads) : tasks_(num_threads), threads_() {
            for (size_t i = 0; i < num_threads; i++)
                threads_.emplace_back([this]() { worker_loop(); });
        }
    
        void submit(std::function<void()> task) {
            tasks_.put(std::move(task));
        }
    
        // join, ~ThreadPool, etc...
    
    private:
        void worker_loop() {
            std::optional<std::function<void()>> task;
            while (!tasks_.is_closed() && (task = tasks_.take()).has_value()) {
                std::invoke(*task);
            }
        }
    
    private:
        BlockingQueue<std::function<void ()>> tasks_;
        std::vector<std::thread> threads_;
    };
    ```
    С какой проблемой синхронизации вы бы столкнулись, если бы решили реализовать параллельный quick sort с помощью такого пула?

    ```cpp
    int* separate_by_pivot(int* begin, int* pivot, int* end) {
        // put elements less than *pivot to the left,
        // and elements greater than *pivot to the right
        // return new position of *pivot
    }
    
    void parallel_sort(ThreadPool& pool, int* begin, int* end) {
        if (end - begin < 2)
            return;
    
        int* med = begin + (end - begin) / 2;
        med = separate_by_pivot(begin, med, end);
        pool.submit([&pool, begin, med] { parallel_sort(pool, begin, med); })
        pool.submit([&pool, med, end]   { parallel_sort(pool, med, end); })
    }
    ```

    Как её можно исправить?

2. Почему в интерфейсе очереди нет метода `is_empty`?

