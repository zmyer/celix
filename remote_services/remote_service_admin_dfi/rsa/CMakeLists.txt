# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
# 
#   http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.

include_directories(
    private/include
    ${PROJECT_SOURCE_DIR}/utils/public/include
    ${PROJECT_SOURCE_DIR}/log_service/public/include
    ${PROJECT_SOURCE_DIR}/remote_services/utils/private/include
    ${PROJECT_SOURCE_DIR}/remote_services/remote_service_admin/public/include
    ${PROJECT_SOURCE_DIR}/remote_services/remote_service_admin/private/include
    ${PROJECT_SOURCE_DIR}/remote_services/remote_service_admin_http/private/include
    ${PROJECT_SOURCE_DIR}/dfi/public/include
    ${PROJECT_SOURCE_DIR}/remote_services/examples/calculator_service/public/include
)

add_bundle(remote_service_admin_dfi 
    VERSION 0.0.1
    SYMBOLIC_NAME "apache_celix_remote_service_admin_dfi"
    NAME "Apache Celix Remote Service Admin Dynamic Function Interface (DFI)"
    SOURCES
    private/src/remote_service_admin_dfi.c
    private/src/remote_service_admin_activator.c
    private/src/export_registration_dfi.c
    private/src/import_registration_dfi.c
    private/src/dfi_utils.c

    ${PROJECT_SOURCE_DIR}/remote_services/remote_service_admin/private/src/endpoint_description.c

    ${PROJECT_SOURCE_DIR}/remote_services/utils/private/src/civetweb.c
    ${PROJECT_SOURCE_DIR}/log_service/public/src/log_helper.c
)
target_link_libraries(remote_service_admin_dfi celix_framework celix_utils celix_dfi ${CURL_LIBRARIES} ${JANSSON_LIBRARIES})

install_bundle(remote_service_admin_dfi)
