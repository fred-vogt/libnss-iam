/*
   Copyright 2010-2017 Amazon.com, Inc. or its affiliates. All Rights Reserved.

   This file is licensed under the Apache License, Version 2.0 (the "License").
   You may not use this file except in compliance with the License. A copy of
   the License is located at

    http://aws.amazon.com/apache2.0/

   This file is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied. See the License for the
   specific language governing permissions and limitations under the License.
*/
#include <aws/core/Aws.h>
#include <aws/iam/IAMClient.h>
#include <aws/iam/model/GetUserRequest.h>
#include <aws/iam/model/GetUserResult.h>
#include <stdio.h>
#include <pwd.h>

static const char* DATE_FORMAT = "%Y-%m-%d";
Aws::IAM::Model::User get_iam_user(char *username);
unsigned long hash(const char *str);
int get_posix_iam_user(char *buffer, int buflen, struct passwd *p);

/**
 * Lists all iam users
 */
#ifndef TEST
int main(int argc, char** argv)
{
	char buffer[1024];
	struct passwd p;
	strcpy(buffer, argv[1]);

	if(get_posix_iam_user(buffer, 1024, &p))
		printf("%s\n", p.pw_name);
}
#endif
#define __LEN 64
int get_posix_iam_user(char *buffer, int buflen, struct passwd *p) {
	
	Aws::IAM::Model::User iam_user = get_iam_user(buffer);
	if (!iam_user.GetUserId().empty()) {
		if (p) {
			char home[] = "/home/iam-user";
			char shell[] = "/bin/bash";
			char pwd[] = "*";
			int bytes = iam_user.GetUserName().size()+1 > __LEN ? __LEN : iam_user.GetUserName().size()+1, offset = 0;
			p->pw_name = memcpy (buffer + offset, iam_user.GetUserName().c_str(), bytes);
			offset += bytes;
			p->pw_uid = hash(iam_user.GetUserId().c_str());
			p->pw_gid = 0;
			bytes = iam_user.GetArn().size()+1 > __LEN ? __LEN : iam_user.GetArn().size()+1;
			p->pw_gecos = memcpy (buffer + offset, iam_user.GetArn().c_str(), bytes);
			offset += bytes;
			bytes = sizeof(home) > __LEN ? __LEN : sizeof(home); 
			p->pw_dir = memcpy (buffer + offset, home, bytes); 
			offset += bytes;
			bytes = sizeof(shell) > __LEN ? __LEN : sizeof(shell); 
			p->pw_shell = memcpy (buffer + offset, shell, bytes); 
			offset += bytes;
			bytes = sizeof(pwd) > __LEN ? __LEN : sizeof(pwd); 
			p->pw_passwd = memcpy (buffer + offset, "", buflen - offset);
		}
		return 1;	
	}
	return 0;	
}

Aws::IAM::Model::User get_iam_user(char *username) {
    Aws::SDKOptions options;
    Aws::InitAPI(options);

    {
        Aws::IAM::IAMClient iam;
	Aws::IAM::Model::GetUserRequest get_request;
	get_request.SetUserName(username);

            auto outcome = iam.GetUser(get_request);
            if (!outcome.IsSuccess()) {
                std::cout << "Failed to list iam users:" <<
                    outcome.GetError().GetMessage() << std::endl;
            }
    	Aws::ShutdownAPI(options);
	return outcome.GetResult().GetUser();

    }
}
