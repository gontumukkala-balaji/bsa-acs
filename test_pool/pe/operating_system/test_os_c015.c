/** @file
 * Copyright (c) 2024, Arm Limited or its affiliates. All rights reserved.
 * SPDX-License-Identifier : Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 **/
#include "val/common/include/acs_val.h"
#include "val/common/include/acs_pe.h"

#define TEST_NUM   (ACS_PE_TEST_NUM_BASE  +  15)
#define TEST_RULE  "B_PE_14"
#define TEST_DESC  "Check SVE2 if PE is Armv9             "

static
void
payload()
{
    uint32_t pe_family;
    uint64_t data;
    uint32_t primary_pe_idx = val_pe_get_primary_index();
    uint32_t index = val_pe_get_index_mpid(val_pe_get_mpid());

    pe_family = val_get_pe_architecture(index);

    /* If processor is not Armv9, FEAT_SVE2 is not required */
    if (pe_family != PROCESSOR_FAMILY_ARMV9) {
        val_set_status(index, RESULT_SKIP(TEST_NUM, 1));
        return;
    }

    /* Read ID_AA64ZFR0_EL1 for SVE2 support */
    data = val_pe_reg_read(ID_AA64ZFR0_EL1);
    if (index == primary_pe_idx)
        val_print(ACS_PRINT_DEBUG, "\n       ID_AA64ZFR0_EL1 = %llx", data);

    /* For Armv9, the ID_AA64ZFR0_EL1.SVEver, bits [3:0] value 0b0000 is not permitted */
    if (VAL_EXTRACT_BITS(data, 0, 3) == 0) {
        val_set_status(index, RESULT_FAIL(TEST_NUM, 1));
        return;
    }

    val_set_status(index, RESULT_PASS(TEST_NUM, 1));
}

uint32_t
os_c015_entry(uint32_t num_pe)
{
    uint32_t status = ACS_STATUS_FAIL;

    status = val_initialize_test(TEST_NUM, TEST_DESC, num_pe);
    /* This check is when user is forcing us to skip this test */
    if (status != ACS_STATUS_SKIP)
        val_run_test_payload(TEST_NUM, num_pe, payload, 0);

    /* get the result from all PE and check for failure */
    status = val_check_for_error(TEST_NUM, num_pe, TEST_RULE);
    val_report_status(0, ACS_END(TEST_NUM), NULL);

    return status;
}
