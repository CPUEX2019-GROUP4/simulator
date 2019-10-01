CC = g++
CFLAGS = -Wall
#LDFLAGS = -lpthread -lm
TARGET = sim
SRCS = sim.cc
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.dep)
RM = rm -f


$(TARGET) : $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

#依存関係の自動読み込み、生成
#%.dep:%.c
#	$(SHELL) -c '$(CC) -MM $< > $@'

#依存関係を出力したファイル(*.dep)をインクルード
#-include $(DEPS)

clean:
	$(RM) $(TARGET) $(DEPS) $(OBJS)

swap:
	gcc -Wall swap.c -o swap
