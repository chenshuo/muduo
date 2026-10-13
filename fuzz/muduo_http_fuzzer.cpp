/*  Copyright 2021 Google LLC
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
      http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "muduo/net/http/HttpContext.h"
#include "muduo/net/Buffer.h"
#include "muduo/base/Timestamp.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size){
	char *new_str = (char *)malloc(size+1);
	if (new_str == NULL){
		return 0;
	}
	memcpy(new_str, data, size);
	new_str[size] = '\0';
	
	muduo::net::HttpContext context;
	muduo::net::Buffer input;
	input.append(new_str);
	context.parseRequest(&input, muduo::Timestamp::now());
	free(new_str);
	return 0;
}
