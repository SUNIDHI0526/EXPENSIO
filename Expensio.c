#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#define USERS_FILE "users.txt"
#define TX_FILE "transactions.txt"
#define LOANS_FILE "loans.txt"
#define MAX 256
#define CAPTCHA_LEN 6
void read_line(char *buf, int size) {
	if (fgets(buf, size, stdin)) {
		buf[strcspn(buf, "\n")] = 0;
	} else {
		buf[0] = '\0';
	}
}
void draw_box(const char *title) {
	int width = 66;
	int pad = (width - 2 - (int)strlen(title)) / 2;
	for (int i = 0; i < width; i++) putchar('*');
	printf("\n*%*s%s%*s*\n", pad, "", title, width - 2 - pad - (int)strlen(title), "");
	for (int i = 0; i < width; i++) putchar('*');
	printf("\n\n");
}
int password_strength(const char *pw) {
	int hasLower = 0, hasUpper = 0, hasDigit = 0, hasSpecial = 0;
	int len = (int)strlen(pw);
	for (int i = 0; pw[i]; i++) {
		if (islower((unsigned char)pw[i])) hasLower = 1;
		else if (isupper((unsigned char)pw[i])) hasUpper = 1;
		else if (isdigit((unsigned char)pw[i])) hasDigit = 1;
		else hasSpecial = 1;
	}
	int score = 0;
	if (len >= 8) score++;
	if (hasLower) score++;
	if (hasUpper) score++;
	if (hasDigit) score++;
	if (hasSpecial) score++;
	return score;
}
void generate_captcha(char *cap) {
	const char chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
	for (int i = 0; i < CAPTCHA_LEN; i++)
		cap[i] = chars[rand() % (int)(sizeof(chars) - 1)];
	cap[CAPTCHA_LEN] = '\0';
}
int user_exists(const char *userid) {
	FILE *fp = fopen(USERS_FILE, "r");
	if (!fp) return 0;
	char line[MAX];
	char stored[MAX];
	while (fgets(line, sizeof(line), fp)) {
		if (strncmp(line, "USER_ID:", 8) == 0) {
			if (sscanf(line, "USER_ID: %255s", stored) == 1) {
				if (strcmp(stored, userid) == 0) {
					fclose(fp);
					return 1;
				}
			}
		}
	}
	fclose(fp);
	return 0;
}
void save_user(const char *id, const char *email, const char *pw,
               const char *phone, const char *dob, const char *cap)
{
	FILE *fp = fopen(USERS_FILE, "a");
	if (!fp) {
		perror("Unable to open users file");
		return;
	}
	fprintf(fp, "USER_START\n");
	fprintf(fp, "USER_ID: %s\n", id);
	fprintf(fp, "EMAIL: %s\n", email);
	fprintf(fp, "PASSWORD: %s\n", pw);
	fprintf(fp, "PHONE: %s\n", phone);
	fprintf(fp, "DOB: %s\n", dob);
	fprintf(fp, "CAPTCHA: %s\n", cap);
	fprintf(fp, "USER_END\n\n");
	fclose(fp);
}
int verify_login(const char *id, const char *email, const char *pw) {
	FILE *fp = fopen(USERS_FILE, "r");
	if (!fp) return 0;
	char line[MAX], u[MAX], e[MAX], p[MAX];
	while (fgets(line, sizeof(line), fp)) {
		if (strncmp(line, "USER_ID:", 8) == 0) {
			if (sscanf(line, "USER_ID: %255s", u) != 1) continue;
			if (!fgets(line, sizeof(line), fp)) break;
			if (sscanf(line, "EMAIL: %255s", e) != 1) continue;
			if (!fgets(line, sizeof(line), fp)) break;
			if (sscanf(line, "PASSWORD: %255s", p) != 1) continue;
			if (strcmp(u, id) == 0 && strcmp(e, email) == 0 && strcmp(p, pw) == 0) {
				fclose(fp);
				return 1;
			}
		}
	}
	fclose(fp);
	return 0;
}
void add_transaction(const char *uid, const char *date, const char *category,
                     const char *desc, double amount, int type)
{
	FILE *fp = fopen(TX_FILE, "a");
	if (!fp) {
		perror("Unable to open transactions file");
		return;
	}
	fprintf(fp, "TX_START\n");
	fprintf(fp, "USER_ID: %s\n", uid);
	fprintf(fp, "DATE: %s\n", date);
	fprintf(fp, "CATEGORY: %s\n", category);
	fprintf(fp, "DESCRIPTION: %s\n", desc);
	fprintf(fp, "AMOUNT: %.2f\n", amount);
	fprintf(fp, "TYPE: %d\n", type);
	fprintf(fp, "TX_END\n\n");
	fclose(fp);
}
double compute_balance(const char *uid) {
	FILE *fp = fopen(TX_FILE, "r");
	if (!fp) return 0.0;
	char line[MAX], u[MAX], date[MAX], cat[MAX], desc[MAX];
	double amt = 0.0;
	int type = 0;
	double bal = 0.0;
	while (fgets(line, sizeof(line), fp)) {
		if (strncmp(line, "USER_ID:", 8) == 0) {
			if (sscanf(line, "USER_ID: %255s", u) != 1) continue;
			if (!fgets(line, sizeof(line), fp)) break;
			sscanf(line, "DATE: %255[^\n]", date);
			if (!fgets(line, sizeof(line), fp)) break;
			sscanf(line, "CATEGORY: %255[^\n]", cat);
			if (!fgets(line, sizeof(line), fp)) break;
			sscanf(line, "DESCRIPTION: %255[^\n]", desc);
			if (!fgets(line, sizeof(line), fp)) break;
			sscanf(line, "AMOUNT: %lf", &amt);
			if (!fgets(line, sizeof(line), fp)) break;
			sscanf(line, "TYPE: %d", &type);
			fgets(line, sizeof(line), fp);
			if (strcmp(u, uid) == 0) {
				if (type == 1) bal += amt;
				else bal -= amt;
			}
		}
	}
	fclose(fp);
	return bal;
}
void add_loan(const char *uid, const char *date, double amount, double interest) {
	FILE *fp = fopen(LOANS_FILE, "a");
	if (!fp) {
		perror("Unable to open loans file");
		return;
	}
	fprintf(fp, "LOAN_START\n");
	fprintf(fp, "USER_ID: %s\n", uid);
	fprintf(fp, "DATE: %s\n", date);
	fprintf(fp, "AMOUNT: %.2f\n", amount);
	fprintf(fp, "INTEREST: %.2f\n", interest);
	fprintf(fp, "LOAN_END\n\n");
	fclose(fp);
}
void view_loans(const char *uid) {
	FILE *fp = fopen(LOANS_FILE, "r");
	if (!fp) {
		printf("No loans file.\n");
		return;
	}
	char line[MAX], u[MAX], date[MAX];
	double amt = 0.0, intr = 0.0;
	int found = 0;
	printf("+------------+------------+------------+----------+\n");
	printf("| USER_ID    | DATE       | AMOUNT     | INTEREST |\n");
	printf("+------------+------------+------------+----------+\n");
	while (fgets(line, sizeof(line), fp)) {
		if (strncmp(line, "USER_ID:", 8) == 0) {
			sscanf(line, "USER_ID: %255s", u);
			fgets(line, sizeof(line), fp);
			sscanf(line, "DATE: %255[^\n]", date);
			fgets(line, sizeof(line), fp);
			sscanf(line, "AMOUNT: %lf", &amt);
			fgets(line, sizeof(line), fp);
			sscanf(line, "INTEREST: %lf", &intr);
			fgets(line, sizeof(line), fp);
			if (strcmp(u, uid) == 0) {
				printf("| %-10s | %-10s | %9.2f | %8.2f |\n", u, date, amt, intr);
				found = 1;
			}
		}
	}
	printf("+------------+------------+-----------+----------+\n");
	if (!found) printf("No loans for this user.\n");
	fclose(fp);
}
void print_table_header() {
	printf("+------------+--------------+---------------------------+-----------+------+\n");
	printf("| DATE       | CATEGORY     | DESCRIPTION               | AMOUNT    | TYPE |\n");
	printf("+------------+--------------+---------------------------+-----------+------+\n");
}
void print_table_footer() {
	printf("+------------+--------------+---------------------------+----------+------+\n");
}
int list_transactions_for_user(char dates[][32], char cats[][64], char descs[][256],
                               double amts[], int types[], int max_entries, const char *uid)
{
	FILE *fp = fopen(TX_FILE, "r");
	if (!fp) return 0;
	char line[MAX], u[MAX], date[MAX], cat[MAX], desc[MAX];
	double amt = 0.0;
	int type = 0;
	int idx = 0;
	while (fgets(line, sizeof(line), fp)) {
		if (strncmp(line, "USER_ID:", 8) == 0) {
			if (sscanf(line, "USER_ID: %255s", u) != 1) continue;
			if (!fgets(line, sizeof(line), fp)) break;
			sscanf(line, "DATE: %255[^\n]", date);
			if (!fgets(line, sizeof(line), fp)) break;
			sscanf(line, "CATEGORY: %255[^\n]", cat);
			if (!fgets(line, sizeof(line), fp)) break;
			sscanf(line, "DESCRIPTION: %255[^\n]", desc);
			if (!fgets(line, sizeof(line), fp)) break;
			sscanf(line, "AMOUNT: %lf", &amt);
			if (!fgets(line, sizeof(line), fp)) break;
			sscanf(line, "TYPE: %d", &type);
			fgets(line, sizeof(line), fp);
			if (strcmp(u, uid) == 0 && idx < max_entries) {
				strncpy(dates[idx], date, 31);
				dates[idx][31] = 0;
				strncpy(cats[idx], cat, 63);
				cats[idx][63] = 0;
				strncpy(descs[idx], desc, 255);
				descs[idx][255] = 0;
				amts[idx] = amt;
				types[idx] = type;
				idx++;
			}
		}
	}
	fclose(fp);
	return idx;
}
void edit_delete_transaction(const char *uid) {
	const int MAX_ENTRIES = 1000;
	char dates[MAX_ENTRIES][32], cats[MAX_ENTRIES][64], descs[MAX_ENTRIES][256];
	double amts[MAX_ENTRIES];
	int types[MAX_ENTRIES];
	int n = list_transactions_for_user(dates, cats, descs, amts, types, MAX_ENTRIES, uid);
	if (n == 0) {
		printf("No transactions to edit/delete.\n");
		return;
	}
	printf("+----+------------+--------------+---------------------------+-----------+------+\n");
	printf("| No | DATE       | CATEGORY     | DESCRIPTION               | AMOUNT    | TYPE |\n");
	printf("+----+------------+--------------+---------------------------+-----------+------+\n");
	for (int i = 0; i < n; ++i) {
		printf("| %2d | %-10s | %-12s | %-25s | %8.2f |  %c   |\n",
		       i + 1, dates[i], cats[i], descs[i], amts[i], types[i] == 1 ? '+' : '-');
	}
	printf("+----+------------+--------------+---------------------------+-----------+------+\n");
	int sel;
	printf("Enter transaction number to edit/delete (0 to cancel): ");
	if (scanf("%d", &sel) != 1) {
		while (getchar() != '\n');
		printf("Invalid input.\n");
		return;
	}
	while (getchar() != '\n');
	if (sel <= 0 || sel > n) {
		printf("Canceled or invalid index.\n");
		return;
	}
	printf("Selected #%d: %s | %s | %s | %.2f | %c\n", sel, dates[sel-1], cats[sel-1], descs[sel-1],
	       amts[sel-1], types[sel-1] == 1 ? '+' : '-');
	char choice[8];
	printf("Enter 'D' to delete/ 'E' to edit/ anything else to cancel: ");
	read_line(choice, sizeof(choice));
	if (choice[0] == 'D' || choice[0] == 'd') {
		FILE *fp = fopen(TX_FILE, "r");
		FILE *tmp = fopen("tx_tmp.txt", "w");
		if (!fp || !tmp) {
			perror("File error >__< ");
			if (fp) fclose(fp);
			if (tmp) fclose(tmp);
			return;
		}
		char line[MAX], u[MAX], date[MAX], cat[MAX], desc[MAX];
		double amt = 0.0;
		int type = 0;
		int idx = 0;
		while (fgets(line, sizeof(line), fp)) {
			if (strncmp(line, "USER_ID:", 8) == 0) {
				char block[4096] = {0};
				strcat(block, line);
				if (!fgets(line, sizeof(line), fp)) break;
				strcat(block, line);
				if (!fgets(line, sizeof(line), fp)) break;
				strcat(block, line);
				if (!fgets(line, sizeof(line), fp)) break;
				strcat(block, line);
				if (!fgets(line, sizeof(line), fp)) break;
				strcat(block, line);
				if (!fgets(line, sizeof(line), fp)) break;
				strcat(block, line);
				if (!fgets(line, sizeof(line), fp)) break;
				strcat(block, line);
				sscanf(block, "USER_ID: %255s", u);
				if (strcmp(u, uid) == 0) {
					idx++;
					if (idx == sel) {
						continue;
					}
				}
				fputs(block, tmp);
			} else {
				fputs(line, tmp);
			}
		}
		fclose(fp);
		fclose(tmp);
		remove(TX_FILE);
		rename("tx_tmp.txt", TX_FILE);
		printf("Transaction deleted ✓.\n");
		return;
	} else if (choice[0] == 'E' || choice[0] == 'e') {
		char new_date[32], new_cat[64], new_desc[256];
		double new_amt;
		int new_type;
		printf("Leave blank to keep existing value.\n");
		printf("New Date (current: %s): ", dates[sel-1]);
		read_line(new_date, sizeof(new_date));
		if (new_date[0] == '\0') strcpy(new_date, dates[sel-1]);
		printf("New Category (current: %s): ", cats[sel-1]);
		read_line(new_cat, sizeof(new_cat));
		if (new_cat[0] == '\0') strcpy(new_cat, cats[sel-1]);
		printf("New Description (current: %s): ", descs[sel-1]);
		read_line(new_desc, sizeof(new_desc));
		if (new_desc[0] == '\0') strcpy(new_desc, descs[sel-1]);
		char amt_buf[64];
		printf("New Amount (current: %.2f): ", amts[sel-1]);
		read_line(amt_buf, sizeof(amt_buf));
		if (amt_buf[0] == '\0') new_amt = amts[sel-1];
		else new_amt = atof(amt_buf);
		char type_buf[8];
		printf("New Type (1=credit,0=debit) (current: %d): ", types[sel-1]);
		read_line(type_buf, sizeof(type_buf));
		if (type_buf[0] == '\0') new_type = types[sel-1];
		else new_type = atoi(type_buf);
		FILE *fp = fopen(TX_FILE, "r");
		FILE *tmp = fopen("tx_tmp.txt", "w");
		if (!fp || !tmp) {
			perror("File error");
			if (fp) fclose(fp);
			if (tmp) fclose(tmp);
			return;
		}
		char line[MAX], block[4096];
		int idx = 0;
		while (fgets(line, sizeof(line), fp)) {
			if (strncmp(line, "USER_ID:", 8) == 0) {
				block[0] = '\0';
				strcat(block, line);
				if (!fgets(line, sizeof(line), fp)) break;
				strcat(block, line);
				if (!fgets(line, sizeof(line), fp)) break;
				strcat(block, line);
				if (!fgets(line, sizeof(line), fp)) break;
				strcat(block, line);
				if (!fgets(line, sizeof(line), fp)) break;
				strcat(block, line);
				if (!fgets(line, sizeof(line), fp)) break;
				strcat(block, line);
				if (!fgets(line, sizeof(line), fp)) break;
				strcat(block, line);
				char owner[MAX];
				sscanf(block, "USER_ID: %255s", owner);
				if (strcmp(owner, uid) == 0) {
					idx++;
					if (idx == sel) {
						fprintf(tmp, "TX_START\n");
						fprintf(tmp, "USER_ID: %s\n", uid);
						fprintf(tmp, "DATE: %s\n", new_date);
						fprintf(tmp, "CATEGORY: %s\n", new_cat);
						fprintf(tmp, "DESCRIPTION: %s\n", new_desc);
						fprintf(tmp, "AMOUNT: %.2f\n", new_amt);
						fprintf(tmp, "TYPE: %d\n", new_type);
						fprintf(tmp, "TX_END\n\n");
						continue;
					}
				}
				fputs(block, tmp);
			} else {
				fputs(line, tmp);
			}
		}
		fclose(fp);
		fclose(tmp);
		remove(TX_FILE);
		rename("tx_tmp.txt", TX_FILE);
		printf("Transaction edited and saved ✓.\n");
		return;
	} else {
		printf("Canceled ✓.\n");
	}
}
void passbook_full(const char *uid) {
	FILE *fp = fopen(TX_FILE, "r");
	if (!fp) {
		printf("No transactions.\n");
		return;
	}
	char line[MAX], u[MAX], date[MAX], cat[MAX], desc[MAX];
	double amt;
	int type;
	int found = 0;
	print_table_header();
	while (fgets(line, sizeof(line), fp)) {
		if (strncmp(line, "USER_ID:", 8) == 0) {
			sscanf(line, "USER_ID: %255s", u);
			fgets(line, sizeof(line), fp);
			sscanf(line, "DATE: %255[^\n]", date);
			fgets(line, sizeof(line), fp);
			sscanf(line, "CATEGORY: %255[^\n]", cat);
			fgets(line, sizeof(line), fp);
			sscanf(line, "DESCRIPTION: %255[^\n]", desc);
			fgets(line, sizeof(line), fp);
			sscanf(line, "AMOUNT: %lf", &amt);
			fgets(line, sizeof(line), fp);
			sscanf(line, "TYPE: %d", &type);
			fgets(line, sizeof(line), fp); /* TX_END */
			if (strcmp(u, uid) == 0) {
				printf("| %-10s | %-12s | %-25s | %8.2f |   %c  |\n",
				       date, cat, desc, amt, type == 1 ? '+' : '-');
				found = 1;
			}
		}
	}
	print_table_footer();
	if (!found) printf("No transactions found.\n");
	fclose(fp);
}
void passbook_yearly(const char *uid, const char *year) {
	FILE *fp = fopen(TX_FILE, "r");
	if (!fp) {
		printf("No transactions.\n");
		return;
	}
	char line[MAX], u[MAX], date[MAX], cat[MAX], desc[MAX];
	double amt;
	int type;
	int found = 0;
	print_table_header();
	while (fgets(line, sizeof(line), fp)) {
		if (strncmp(line, "USER_ID:", 8) == 0) {
			sscanf(line, "USER_ID: %255s", u);
			fgets(line, sizeof(line), fp);
			sscanf(line, "DATE: %255[^\n]", date);
			fgets(line, sizeof(line), fp);
			sscanf(line, "CATEGORY: %255[^\n]", cat);
			fgets(line, sizeof(line), fp);
			sscanf(line, "DESCRIPTION: %255[^\n]", desc);
			fgets(line, sizeof(line), fp);
			sscanf(line, "AMOUNT: %lf", &amt);
			fgets(line, sizeof(line), fp);
			sscanf(line, "TYPE: %d", &type);
			fgets(line, sizeof(line), fp);
			if (strcmp(u, uid) == 0) {
				if (strncmp(date, year, strlen(year)) == 0) {
					printf("| %-10s | %-12s | %-25s | %8.2f |   %c  |\n",
					       date, cat, desc, amt, type == 1 ? '+' : '-');
					found = 1;
				}
			}
		}
	}
	print_table_footer();
	if (!found) printf("No transactions for year %s.\n", year);
	fclose(fp);
}
void passbook_monthly(const char *uid, const char *ym) {
	FILE *fp = fopen(TX_FILE, "r");
	if (!fp) {
		printf("No transactions.\n");
		return;
	}
	char line[MAX], u[MAX], date[MAX], cat[MAX], desc[MAX];
	double amt;
	int type;
	int found = 0;
	print_table_header();
	while (fgets(line, sizeof(line), fp)) {
		if (strncmp(line, "USER_ID:", 8) == 0) {
			sscanf(line, "USER_ID: %255s", u);
			fgets(line, sizeof(line), fp);
			sscanf(line, "DATE: %255[^\n]", date);
			fgets(line, sizeof(line), fp);
			sscanf(line, "CATEGORY: %255[^\n]", cat);
			fgets(line, sizeof(line), fp);
			sscanf(line, "DESCRIPTION: %255[^\n]", desc);
			fgets(line, sizeof(line), fp);
			sscanf(line, "AMOUNT: %lf", &amt);
			fgets(line, sizeof(line), fp);
			sscanf(line, "TYPE: %d", &type);
			fgets(line, sizeof(line), fp);
			if (strcmp(u, uid) == 0) {
				if (strncmp(date, ym, strlen(ym)) == 0) {
					printf("| %-10s | %-12s | %-25s | %8.2f |   %c  |\n",
					       date, cat, desc, amt, type == 1 ? '+' : '-');
					found = 1;
				}
			}
		}
	}
	print_table_footer();
	if (!found) printf("No transactions for %s.\n", ym);
	fclose(fp);
}
void passbook_category(const char *uid, const char *category_query) {
	FILE *fp = fopen(TX_FILE, "r");
	if (!fp) {
		printf("No transactions.\n");
		return;
	}
	char line[MAX], u[MAX], date[MAX], cat[MAX], desc[MAX];
	double amt;
	int type;
	int found = 0;
	print_table_header();
	while (fgets(line, sizeof(line), fp)) {
		if (strncmp(line, "USER_ID:", 8) == 0) {
			sscanf(line, "USER_ID: %255s", u);
			fgets(line, sizeof(line), fp);
			sscanf(line, "DATE: %255[^\n]", date);
			fgets(line, sizeof(line), fp);
			sscanf(line, "CATEGORY: %255[^\n]", cat);
			fgets(line, sizeof(line), fp);
			sscanf(line, "DESCRIPTION: %255[^\n]", desc);
			fgets(line, sizeof(line), fp);
			sscanf(line, "AMOUNT: %lf", &amt);
			fgets(line, sizeof(line), fp);
			sscanf(line, "TYPE: %d", &type);
			fgets(line, sizeof(line), fp); /* TX_END */
			if (strcmp(u, uid) == 0) {
				if (strcasestr(cat, category_query) != NULL) {
					printf("| %-10s | %-12s | %-25s | %8.2f |   %c  |\n",
					       date, cat, desc, amt, type == 1 ? '+' : '-');
					found = 1;
				}
			}
		}
	}
	print_table_footer();
	if (!found) printf("No transactions for category '%s'.\n", category_query);
	fclose(fp);
}
void search_transactions(const char *uid) {
	int ch;
	char query[MAX];
	printf("Search by:\n1.Date\n2.Category\n3.Amount\n4.Description\nEnter choice: ");
	if (scanf("%d", &ch) != 1) {
		while (getchar() != '\n');
		printf("Invalid input.\n");
		return;
	}
	while (getchar() != '\n');
	printf("Enter search text: ");
	read_line(query, sizeof(query));
	FILE *fp = fopen(TX_FILE, "r");
	if (!fp) {
		printf("No data.\n");
		return;
	}
	char line[MAX], u[MAX], date[MAX], cat[MAX], desc[MAX];
	double amt;
	int type;
	int found = 0;
	print_table_header();
	while (fgets(line, sizeof(line), fp)) {
		if (strncmp(line, "USER_ID:", 8) == 0) {
			sscanf(line, "USER_ID: %255s", u);
			fgets(line, sizeof(line), fp);
			sscanf(line, "DATE: %255[^\n]", date);
			fgets(line, sizeof(line), fp);
			sscanf(line, "CATEGORY: %255[^\n]", cat);
			fgets(line, sizeof(line), fp);
			sscanf(line, "DESCRIPTION: %255[^\n]", desc);
			fgets(line, sizeof(line), fp);
			sscanf(line, "AMOUNT: %lf", &amt);
			fgets(line, sizeof(line), fp);
			sscanf(line, "TYPE: %d", &type);
			fgets(line, sizeof(line), fp); 
			if (strcmp(u, uid) != 0) continue;
			int match = 0;
			if (ch == 1 && strstr(date, query)) match = 1;
			if (ch == 2 && strcasestr(cat, query)) match = 1;
			if (ch == 3) {
				double q = atof(query);
				if (q != 0.0) {
					double diff = amt - q;
					if (diff < 0) diff = -diff;
					if (diff < 0.001) match = 1;
				} else {
					if (strstr((char[32]) {
					0
				}, query)) match = 0;
				}
			}
			if (ch == 4 && strcasestr(desc, query)) match = 1;
			if (match) {
				printf("| %-10s | %-12s | %-25s | %8.2f |   %c  |\n",
				       date, cat, desc, amt, type == 1 ? '+' : '-');
				found = 1;
			}
		}
	}
	print_table_footer();
	if (!found) printf("No matching transactions.\n");
	fclose(fp);
}
int main() {///MAIN PROGRAM
	srand((unsigned int)time(NULL));
	while (1) {
		draw_box("EXPENSIO ^o^");
		draw_box("SIGN up    Log in");
		printf("1. Sign up\n2. Log in\n3. EXIT\nEnter choice: ");
		int opt;
		if (scanf("%d", &opt) != 1) {
			while (getchar() != '\n');
			printf("Invalid input.\n");
			continue;
		}
		while (getchar() != '\n');
		if (opt == 3) {
			printf("Exiting EXPENSIO...\nGoodbye! ️(◔◡◔)\n");
			break;
		}
		char uid[MAX], email[MAX], pw[MAX], phone[MAX], dob[MAX], cap[20];
		if (opt == 1) {
			while (1) {
				printf("Enter user id : ");
				read_line(uid, sizeof(uid));
				int ok = 1;
				for (int i = 0; uid[i]; i++) if (!isalnum((unsigned char)uid[i])) ok = 0;
				if (!ok) {
					printf("Only alphanumeric allowed.\n");
					continue;
				}
				if (user_exists(uid)) {
					printf("User exists.\n");
					continue;
				}
				break;
			}

			printf("Email: ");
			read_line(email, sizeof(email));
			while (1) {
				printf("Create password: ");
				read_line(pw, sizeof(pw));
				int score = password_strength(pw);
				printf("Password strength: %d/5\n", score);
				if (score < 4) printf("Too weak, try again.\n");
				else break;
			}
			printf("Phone: ");
			read_line(phone, sizeof(phone));
			printf("DOB (YYYY-MM-DD): ");
			read_line(dob, sizeof(dob));
			generate_captcha(cap);
			printf("CAPTCHA: %s\nEnter captcha: ", cap);
			char cap_in[20];
			read_line(cap_in, sizeof(cap_in));
			if (strcmp(cap, cap_in) != 0) {
				printf("Captcha mismatch. Signup failed.\n");
				continue;
			}
			save_user(uid, email, pw, phone, dob, cap);
			printf("Signup successful. Please login.\n");
		}
		printf("Login user id: ");
		read_line(uid, sizeof(uid));
		printf("Email: ");
		read_line(email, sizeof(email));
		printf("Password: ");
		read_line(pw, sizeof(pw));
		if (!verify_login(uid, email, pw)) {
			printf("Login failed.\n");
			continue;
		}
		printf("\nAccess granted ✓.\n**********Welcome \"%s\"!*************\n", uid);
		int run = 1;
		while (run) {
			int ch;
			printf("\n1.Check balance\n2.Add income/expense\n3.Add loan\n4.Edit/Delete expense\n5.Budget\n6.Passbook\n7.Search\n8.View Loans\n9.Logout\nEnter choice: ");
			if (scanf("%d", &ch) != 1) {
				while (getchar() != '\n');
				printf("Invalid input.\n");
				continue;
			}
			while (getchar() != '\n');
			if (ch == 1) {
				printf("Balance: %.2f\n", compute_balance(uid));
			}
			else if (ch == 2) {
				char date[MAX], cat[MAX], desc[MAX];
				double amt;
				int t;
				printf("Date (YYYY-MM-DD): ");
				read_line(date, sizeof(date));
				printf("Category: ");
				read_line(cat, sizeof(cat));
				printf("Description: ");
				read_line(desc, sizeof(desc));
				printf("Amount: ");
				if (scanf("%lf", &amt) != 1) {
					while (getchar() != '\n');
					printf(" ✕ Invalid amount.\n");
					continue;
				}
				while (getchar() != '\n');
				printf("Type (1 credit, 0 debit): ");
				if (scanf("%d", &t) != 1) {
					while (getchar() != '\n');
					printf("Invalid type.\n");
					continue;
				}
				while (getchar() != '\n');
				add_transaction(uid, date, cat, desc, amt, t);
				printf("Transaction added ✓.\n");
			}
			else if (ch == 3) {
				char date[MAX];
				double amt, intr;
				printf("Date: ");
				read_line(date, sizeof(date));
				printf("Amount: ");
				if (scanf("%lf", &amt) != 1) {
					while (getchar() != '\n');
					printf("Invalid amount.\n");
					continue;
				}
				while (getchar() != '\n');
				printf("Interest (percent): ");
				if (scanf("%lf", &intr) != 1) {
					while (getchar() != '\n');
					printf("Invalid interest.\n");
					continue;
				}
				while (getchar() != '\n');
				add_loan(uid, date, amt, intr);
				printf("Loan added.\n");
			}
			else if (ch == 4) {
				edit_delete_transaction(uid);
			}
			else if (ch == 5) {
				char choice[10];
				printf("Budget Options:\nA. Set your own budget\nB. Program recommended budget\nEnter choice: ");
				read_line(choice, sizeof(choice));
				if (strcasecmp(choice, "A") == 0) {
					double b;
					printf("Enter your monthly budget: ");
					if (scanf("%lf", &b) != 1) {
						while (getchar() != '\n');
						printf("Invalid.\n");
					}
					while (getchar() != '\n');
					printf("Budget set: %.2f\n", b);
				} else {
					double income;
					printf("Enter monthly income: ");
					if (scanf("%lf", &income) != 1) {
						while (getchar() != '\n');
						printf("Invalid!!.\n");
					}
					while (getchar() != '\n');
					printf("\nRecommended (50/30/20):\nNeeds: %.2f\nWants: %.2f\nSavings: %.2f\n",
					       income * 0.5, income * 0.3, income * 0.2);
				}
				printf("Current Balance: %.2f\n", compute_balance(uid));
			}
			else if (ch == 6) {
				printf("Passbook:\nA. Full history\nB. Yearly history\nC. Monthly history\nD. Category-wise history\nEnter choice: ");
				char c[10];
				read_line(c, sizeof(c));
				if (strcasecmp(c, "A") == 0) {
					passbook_full(uid);
				} else if (strcasecmp(c, "B") == 0) {
					char year[8];
					printf("Enter year (YYYY): ");
					read_line(year, sizeof(year));
					passbook_yearly(uid, year);
				} else if (strcasecmp(c, "C") == 0) {
					char ym[8];
					printf("Enter month (YYYY-MM): ");
					read_line(ym, sizeof(ym));
					passbook_monthly(uid, ym);
				} else if (strcasecmp(c, "D") == 0) {
					char category_query[64];
					printf("Enter category: ");
					read_line(category_query, sizeof(category_query));
					passbook_category(uid, category_query);
				} else {
					printf("Invalid option!!\n");
				}
			}
			else if (ch == 7) {
				search_transactions(uid);
			}
			else if (ch == 8) {
				view_loans(uid);
			}
			else if (ch == 9) {
				run = 0;
				printf("\nVisit again\n--------------\n\"**Beaware Of Little Expenses. A Small Leak Will Sink A Great Ship !**\n\n\n");
			}
			else {
				printf("Invalid choice!!\n");
			}
		}
	}
	return 0;
}
