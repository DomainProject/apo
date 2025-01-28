from heapq import *

class Actor():
    def __init__(self, id, trace):
        self._id = id
        self._trace = trace
        self._executed = []
        self._pending = []
        self._last_from_trace = -1
        self._last_lvt = 0
        self._last_wct = 0
        
    def next_from_trace(self):
        if len(self._trace) <= self._last_from_trace+1: a = float('inf')
        else: a = self._trace[self._last_from_trace+1][0]
        return a

    def next_from_rcv(self):
        if len(self._pending) == 0 : b = float('inf')
        else: b = self._pending[0][0]
        return b

    def next_ts(self):
        return min(self.next_from_trace(), self.next_from_rcv())

    def get_id(self):
        return self._id

    def curr_ts(self):
        if len(self._trace) <= self._last_from_trace: return float('inf')
        return self._last_lvt

    def rcv(self, ts, wct, actor):
        heappush(self._pending, (ts, wct, actor))

    def do_step(self, wct):
        a = self.next_from_trace()
        b = self.next_from_rcv()
        m = min(a,b)
        
        if len(self._executed) > 0 and m < self._executed[-1]:
            print("OUT OF ORDER EVENT", m, a, b, self._executed, self)
            exit(1)
        self._last_wct = wct
        self._executed +=[m]
        self._last_lvt = m

        if a < b: 
            self._last_from_trace+=1
            return True
        else: 
            self._last_lvt = b
            heappop(self._pending)
            return False

    def fix_executed(self):
        cnt=0
        while self._executed[-1] >= self.next_ts():
            self._executed.pop()
            cnt+=1
        return cnt
    def do_reverse(self):
        self._last_from_trace -= 1

    def fix_bound(self):
        self._last_lvt = max(self._executed[-1], self._trace[self._last_from_trace][0])


    def last_trace(self):
        return self._trace[self._last_from_trace]

    def commit(self, gvt):
        cnt = 0
        last_val = -1
        #print(self._executed)
        while len(self._executed) > 0 and self._executed[0] < gvt: 
            cnt += 1
            tmp = self._executed.pop(0)
            if tmp < last_val: 
                print("LAST EXECUTED OUT OF ORDER", self._executed, tmp, last_val, self, gvt)
                exit(1)
            last_val = tmp
        return cnt

    def __gt__(self, other):
        if self.next_ts() != other.next_ts():
            return self.next_ts() > other.next_ts()
        else:
            return self._last_wct > other._last_wct
    def __lt__(self, other):
        return self.next_ts() < other.next_ts()

    def __str__(self):
        return f"Actor(id:{self._id},lvt:{self._last_lvt},next:{self.next_ts()})"