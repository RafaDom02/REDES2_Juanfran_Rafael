#-----------------------
# Directories for source / object / header files
SRCDIR = ./src
OBJDIR = ./obj
INCLUDE = ./include
RESULTS = ./resultados
#-----------------------
CC = gcc -pthread  
CFLAGS = -g 
IFLAGS = -l:libconfuse.a -I ${INCLUDE}
EXE := server

all : directories $(EXE)
directories: ${INCLUDE} ${OBJDIR} ${SRCDIR} ${RESULTS}

.PHONY : clean directories
clean :
	rm -f ${OBJDIR}/*.o core $(EXE)

$(EXE) : % : ${OBJDIR}/main.o ${OBJDIR}/http.o ${OBJDIR}/picohttpparser.o  ${OBJDIR}/htmlparser.o ${OBJDIR}/threadpool.o ${OBJDIR}/ip.o ${OBJDIR}/conf.o 
	@echo "#---------------------------"
	@echo "# Generating $@ "
	@echo "# Depepends on $^"
	@echo "# Has changed $<"
	$(CC) $(CFLAGS) -o $@ ${OBJDIR}/main.o ${OBJDIR}/http.o ${OBJDIR}/picohttpparser.o ${OBJDIR}/htmlparser.o ${OBJDIR}/threadpool.o ${OBJDIR}/ip.o ${OBJDIR}/conf.o ${IFLAGS}
	rm -f ${OBJDIR}/*.o


${OBJDIR}/%.o: ${SRCDIR}/%.c
	@echo "#--------------------"
	@echo "# Generating $@ "
	@echo "# Depends on $^ "
	@echo "# Has changed $<"
	$(CC) $(CFLAGS) -o $@ -c $< $(IFLAGS)


#-----------------------
# Rules for creating directories and avoid crashing
${SRCDIR}:
	@mkdir ${SRCDIR}
${OBJDIR}:
	@mkdir ${OBJDIR}
${INCLUDE}:
	@mkdir ${INCLUDE}
${RESULTS}:
	@mkdir ${RESULTS}