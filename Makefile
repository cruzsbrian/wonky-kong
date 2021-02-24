CC = g++
CFLAGS = -Wall -flto -Ofast -lpthread
LDFLAGS = -Ofast -lpthread

SRCDIR = src
OBJDIR = obj
BINDIR = bin

BOARD_SRCS = board.cpp
EVAL_SRCS = simple.cpp
SEARCH_SRCS = negamax.cpp
GAME_SRCS = game.cpp
UTIL_SRCS = util.cpp
COMMON_SRCS = $(addprefix board/, $(BOARD_SRCS)) \
			  $(addprefix eval/, $(EVAL_SRCS)) \
			  $(addprefix search/, $(SEARCH_SRCS)) \
			  $(addprefix game/, $(GAME_SRCS)) \
			  $(UTIL_SRCS) \

CLI_SRCS = cli.cpp game/human_player.cpp
CS2_SRCS = cs2.cpp
TRAIN_SRCS = train.cpp

COMMON_OBJS = $(COMMON_SRCS:.cpp=.o)
CLI_OBJS = $(COMMON_OBJS) $(CLI_SRCS:.cpp=.o)
CS2_OBJS = $(COMMON_OBJS) $(CS2_SRCS:.cpp=.o)
TRAIN_OBJS = $(COMMON_OBJS) $(TRAIN_SRCS:.cpp=.o)


all: wonky_cli wonky_cs2 wonky_train

wonky_cli: $(addprefix $(OBJDIR)/, $(CLI_OBJS))
	mkdir -p bin
	$(CC) $^ $(LDFLAGS) -o $(BINDIR)/$@

wonky_cs2: $(addprefix $(OBJDIR)/, $(CS2_OBJS))
	mkdir -p bin
	$(CC) $^ $(LDFLAGS) -o $(BINDIR)/$@

wonky_train: $(addprefix $(OBJDIR)/, $(TRAIN_OBJS))
	mkdir -p bin
	$(CC) $^ $(LDFLAGS) -o $(BINDIR)/$@

$(OBJDIR)/%.o: $(SRCDIR)/$(notdir %.cpp)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BINDIR)/* $(OBJDIR)/*
