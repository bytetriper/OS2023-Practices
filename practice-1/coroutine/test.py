import os
from matplotlib import pyplot as plt
if __name__=="__main__":
    test=[]
    for i in range(1,200,30):
        test_batch = 100
        fail=0
        for epoch in range(test_batch):
            if epoch%10==0:
                print(f"test {i:d} {epoch/test_batch:.2f} finished")
            exit_code=os.system(f"./main {i:d}  > /dev/null 2>&1")
            if exit_code!=0:
                fail+=1
        test.append(fail/test_batch)
        print(f"test {i:d} finished")
        print(f"fail rate: {fail/test_batch:.2f}")
    plt.plot(range(1,200,30),test)
    plt.savefig("./test.png")

