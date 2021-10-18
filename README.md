Параллельные и распределённые вычисления
===========================================

В этом репозитории будут публиковаться условия домашних заданий и тесты для них. Из него же через GitHub Classroom создаются ваши репозитории для сдачи.

# Задачи

1. [blocking-queue](https://github.com/hse-pndc-2021/Tasks/tree/master/blocking-queue)
2. [mutex](https://github.com/hse-pndc-2021/Tasks/tree/master/mutex)

# Подготовка окружения

Поскольку Classroom использует неудобный механизм шаблонов, придётся выполнить дополнительные шаги по настройке окружения.

* Найдите в чате курса ссылку на задание в Classroom и примите его.

* Дождитесь, пока GitHub создаст для вас репозитрий. Он будет называться `Tasks-your-username` и принадлежать организации `hse-pndc-2021`.

* Склонируйте свой репозиторий.

* Перейдите в корень репозитория и выполните следующие команды:

  ```sh
  git remote add super https://github.com/hse-pndc-2021/Tasks.git
  git fetch --all
  git reset --hard super/master
  git push -f origin master
  ```
  **Важно выполнить эти шаги до внесения любых изменений!**

В дальнейшем вы сможете подтягивать изменения из основного репозитория в свой командой

```sh
git pull --rebase super
```

# Запуск тестов

Некоторые задачи содержат автоматические тесты. Тесты рассчитаны на сборку в тулчейне gcc / clang, make, ctest. В Windows можно использовать WSL или виртуальную машину с Linux. Можно попытаться собрать тесты компилятором msvc, но результат не гарантирован.

Убедитесь, что у вас установлены

* gcc 10.2+ или clang 10+
* cmake 3.15+

В gcc возможны ложные срабатывания thread sanitizer'а. Если вы с этим столкнулись, попробуйте clang.

После этого собрать тесты для задачи `task-name` можно так:

```sh
mkdir build && cd build
cmake -DTASK=task-name .. 
make
ctest --verbose
```

### Опции

Собрать без санитайзеров

```sh
cmake -DDISABLE_SANITIZERS=ON -DTASK=task-name .. 
```

Собрать тесты всех задач

```sh
cmake ..
```

Собрать другим компилятором

```sh
CXX=clang-11 cmake -DTASK=task-name ..
```

# Сдача заданий

* Создайте ветку `task-name`, где `task-name` &mdash; название задачи.
* Закомитте в неё файлы с решением и сделайте пуш в свой репозиторий.

```sh
git add task-name/file.cpp
git commit -m "Message does not matter"
git push -u origin task-name
```

* Откройте pull request в master.
* О новых реквестах приходят уведомления, писать в личку преподавателю не надо.

# Оценивание

Для каждой задачи будет указан дедлайн и баллы. Можно сдавать задачи после дедлайна, но их стоимость уменьшается на 1 балл каждую неделю (1 балл задача теряет сразу после дедлайна), но не может опуститься ниже 1.

Прохождение автоматических тестов не гарантирует ни корректности вашего кода, ни того, что задача будет засчитана. Замечания по ревью кода можно исправлять после дедлайна, исправленные замечания не влияют на оценку.

Формула оценки появится позже.
