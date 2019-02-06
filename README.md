# pipeline project in operating system course
---

## How to use :

For compiling you should first compile worker and presenter and then load_balancer it means:
    gcc worker.cpp -o worker
    gcc presenter.cpp -o presenter
    gcc load_balancer.cpp -o load_balancer

Then for run just use this commands :
    ./load _balancer

## How it works?

In this  project we  used unnamed  and named pipe. we assume that we have a internet shop center and we want to search among the goods in this shop.
the commad should follow this style:
    (<field name> = <corresponding filtering value> - )*( <sorting value = ascend/descend> - )?prc_cnt = n – dir = <relative dataset address>
for example:
    brand = asus - price = descend - prc_cnt = 4 - dir = loptops