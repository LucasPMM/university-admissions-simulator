#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lista.h"

// ------------------ Course Functions Start ------------------ //

// Function to initialize course structure
Course* course_create_course(){
	return NULL;
}

// Function to create an structure of course 
Course* course_insert_course(Course *l, char courseName[100], int positions){
	int i;
	Course *novo = (Course *) calloc (1,sizeof(Course));
	novo->positions = positions;
	strcpy(novo->courseName, courseName);
	novo->courseName[strlen(courseName) - 1] = '\0';
	return novo;
}

// Function to free the occupied memory with the data of the structure course
void course_free_course(Course **c, int courseQtd){
	int i;
	for (i = 0; i < courseQtd; i++) {
		if (c[i]->listOfStudents) {
			lst_free(c[i]->listOfStudents);
		}
		free(c[i]);
	}
}

// Function to read student informations
Course **read_students(Course **listOfCourses) {
	char studentName[100];
	int firstOp, secondOP;
	float score;
	char c;
	while((c = getchar()) != EOF && c != '\n')
		continue;
	fgets(studentName, sizeof(studentName), stdin);

	scanf("%f%d%d", &score, &firstOp, &secondOP);

	listOfCourses[firstOp]->listOfStudents = lst_insert(listOfCourses[firstOp]->listOfStudents, studentName, score, firstOp, secondOP, 0, firstOp);
	listOfCourses[secondOP]->listOfStudents = lst_insert(listOfCourses[secondOP]->listOfStudents, studentName, score, firstOp, secondOP, 0, secondOP);

	return listOfCourses;
}

// Function to read course informations
Course *read_informations(Course *l) {
	char courseName[100];
	int positions;
	char c;
	while((c = getchar()) != EOF && c != '\n')
		continue;
	fgets(courseName, sizeof(courseName), stdin);

	scanf("%d", &positions);
	
	return course_insert_course(l, courseName, positions);
}

void course_print_result(Course *l, int hasProx){
	int waitingListPrinted = 0;

	printf("%s %.2f\n", l->courseName, l->passingScore);
	printf("Classificados\n");
	if (l && l->listOfStudents && l->positions && (l->listOfStudents->firstOp || l->listOfStudents->secondOp)) {

		List *p;
		int i = 0;
		for(p=l->listOfStudents; p!=NULL; p=p->prox){
			printf("%s %.2f\n", p->studentName , p->score);
			if (i + 1 == l->positions || (p->prox == NULL && !waitingListPrinted)) {
				waitingListPrinted = 1;
				printf("Lista de espera\n");
			}
			i++;
		}
		if (hasProx)
			printf("\n");
	} else {
		printf("Lista de espera\n");
	}
}

Course **check_courses_lists(int courseQtd, Course **listOfCourses) {
	int hasChanges = 1, i, j;
	while (hasChanges) {
		hasChanges = 0;
		for (i = 0; i < courseQtd; i++) {
			char *studentToRemove = find_student_name_to_remove(listOfCourses[i]->listOfStudents, listOfCourses[i]->positions, i);
			if (strcmp(studentToRemove, "NULL") != 0) {
				int index = find_student_second_option_index(listOfCourses[i]->listOfStudents, studentToRemove);
				if (index != -1) {
					listOfCourses[index]->listOfStudents = lst_retira(listOfCourses[index]->listOfStudents, studentToRemove);
					hasChanges = 1;
					listOfCourses[i]->listOfStudents = mark_as_second_option_removed(listOfCourses[i]->listOfStudents, studentToRemove);
				}
			}
		}
	}
	return listOfCourses;
}

Course *course_passing_score(Course *c, int index) {
	List *l = c->listOfStudents;
	int positions = c->positions, i;
	
	c->passingScore = 0.0;

	for(i = 0; l != NULL; l = l->prox, i++) {
		if (i + 1 == positions) {
			c->passingScore = l->score;
			break;
		}
	}
	return c;
}

// ------------------ Course Functions End ------------------ //


// ------------------ List Functions Start ------------------ //

void lst_free(List *l){
	List *p = l;
	while(p!=NULL){
		List *t = p->prox;
		free(p);
		p=t;
	}
}

// Function to remove an element from list
List *lst_retira(List *l, char studentName[100]){
	List *ant = NULL;
	List *p=l;
	while(p!=NULL && strcmp(p->studentName, studentName) != 0) {
		ant=p;
		p=p->prox;
	}

	if (p==NULL){
		return l;
	}
	if(ant==NULL) {
		l=p->prox;
	}
	else {
		ant->prox=p->prox;
	}
	free(p);
	return l;
}

// Function to insert elements on list
List *lst_insert(List *l, char studentName[100], float score, int firstOp, int secondOP, int secondOptionWasRemoved, int courseIndex){
	List *novo;
	List *ant = NULL;
	List *p = l;
	int i;

	while(p!=NULL && p->score >= score){
		if (p->score == score && p->secondOp == courseIndex && firstOp == courseIndex) {
			break;
		}
		ant = p;
		p = p->prox;
	}

	novo = (List *) calloc (1,sizeof(List));
	novo->score = score;
	novo->firstOp = firstOp;
	novo->secondOp = secondOP;
	novo->secondOptionWasRemoved = secondOptionWasRemoved;
	strcpy(novo->studentName, studentName);
	novo->studentName[strlen(studentName) - 1] = '\0';

	if (ant == NULL){
		novo->prox = l;
		l = novo;
	}
	else{
		novo->prox = ant->prox;
		ant->prox = novo;
	}
	return l;
}

// Function to find the name of an student that have passed on its first course option
char *find_student_name_to_remove(List *listOfStudents, int positions, int index) {
	int i;
	List *s = listOfStudents;
	for(i = 0; s!=NULL; s=s->prox){
		if (s->firstOp == index && i + 1 <= positions && !s->secondOptionWasRemoved) {
			// remove second option of that student
			return s->studentName;
		}		
		i++;
	}
	return "NULL";
}

// Function to mark the student that have passed on its first course option
List *mark_as_second_option_removed(List *listOfStudents, char studentName[100]) {
	int i;
	List *s = listOfStudents;

	if (strcmp(s->studentName , studentName) == 0) {
		s->secondOptionWasRemoved = 1;
	} else {
		s->prox = mark_as_second_option_removed(s->prox, studentName);
	}
	return s;
}

// Function to update user informations
List *update_user_informations(List *listOfStudents, char oldStudentName[100], char newStudentName[100], int firstOp, int secondOp, int secondOptionWasRemoved, float score) {
	int i;

	if (strcmp(listOfStudents->studentName , oldStudentName) == 0) {
		strcpy(listOfStudents->studentName, newStudentName);
		listOfStudents->firstOp = firstOp;
		listOfStudents->secondOp = secondOp;
		listOfStudents->secondOptionWasRemoved = secondOptionWasRemoved;
		listOfStudents->score = score;		
	} else {
		listOfStudents = update_user_informations(listOfStudents->prox, oldStudentName, newStudentName, firstOp, secondOp, secondOptionWasRemoved, score);
	}
	return listOfStudents;
}

// Function to find the index of list array that contains the student's second course option
int find_student_second_option_index(List *listOfStudents, char studentName[100]){
	int i;
	List *s = listOfStudents;
	for(i = 0; s!=NULL; s = s->prox){
		if (strcmp(s->studentName, studentName) == 0) {
			// remove second option of that student
			return s->secondOp;
		}		
		i++;
	}
	return -1;
}

// ------------------ List Functions End ------------------ //


void init_sisu() {
	int i, courseQtd, studentsQtd, k;

	// Read the quantity of courses and students
	scanf("%d%d", &courseQtd, &studentsQtd);

	// Read all courses and number of positions: O(m)
	Course **listOfCourses = (Course **) calloc (courseQtd,sizeof(Course *));
	for (i = 0; i < courseQtd; i++) {
		listOfCourses[i] = read_informations(listOfCourses[i]);
	}

	// Inset all students on right course list: O(n)
	for (i = 0; i < studentsQtd; i++) {
		listOfCourses = read_students(listOfCourses);	
	}

	// Remove second course option from students that have already passed on first option: O(m*n²)
	listOfCourses = check_courses_lists(courseQtd, listOfCourses);
		
	// Check define passing score: O(m*n)
	for (i = 0; i < courseQtd; i++) {
		listOfCourses[i] = course_passing_score(listOfCourses[i], i);
	}

	// Print all courses and lists: O(m*n)
	for (i = 0; i < courseQtd; i++) {
		int hasProx = 0;
		if (i + 1 < courseQtd) {
			hasProx = 1;
		}
		course_print_result(listOfCourses[i], hasProx);
	}

	// Remove list from memory
	course_free_course(listOfCourses, courseQtd); // O(m*n)
	free(listOfCourses);
}
