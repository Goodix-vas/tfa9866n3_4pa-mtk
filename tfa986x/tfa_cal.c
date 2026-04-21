/*
 * Copyright (C) 2014-2020 NXP Semiconductors, All Rights Reserved.
 * Copyright 2020 GOODIX, All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include "inc/tfa_sysfs.h"

#define TFA_CAL_DEV_NAME	"tfa_cal"
#define FILESIZE_CAL	(10)

/* ---------------------------------------------------------------------- */

static ssize_t SPK1_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t SPK1_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size);
static DEVICE_ATTR_RW(SPK1);

static ssize_t SPK2_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t SPK2_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size);
static DEVICE_ATTR_RW(SPK2);

static ssize_t SPK3_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t SPK3_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size);
static DEVICE_ATTR_RW(SPK3);

static ssize_t SPK4_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t SPK4_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size);
static DEVICE_ATTR_RW(SPK4);

static ssize_t temp0_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t temp0_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size);
static DEVICE_ATTR_RW(temp0);

static ssize_t temp1_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t temp1_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size);
static DEVICE_ATTR_RW(temp1);

static ssize_t temp2_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t temp2_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size);
static DEVICE_ATTR_RW(temp2);

static ssize_t temp3_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t temp3_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size);
static DEVICE_ATTR_RW(temp3);

static ssize_t status_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t status_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size);
static DEVICE_ATTR_RW(status);

static ssize_t ref_temp_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t ref_temp_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size);
static DEVICE_ATTR_RW(ref_temp);

static ssize_t reinit_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t reinit_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size);
static DEVICE_ATTR(reinit, 0664, reinit_show, reinit_store);

static struct attribute *tfa_cal_attr[] = {
	&dev_attr_SPK1.attr,
	&dev_attr_SPK2.attr,
	&dev_attr_SPK3.attr,
	&dev_attr_SPK4.attr,
	&dev_attr_temp0.attr,
	&dev_attr_temp1.attr,
	&dev_attr_temp2.attr,
	&dev_attr_temp3.attr,
	&dev_attr_status.attr,
	&dev_attr_ref_temp.attr,
	&dev_attr_reinit.attr,
	NULL,
};

static struct attribute_group tfa_cal_attr_grp = {
	.attrs = tfa_cal_attr,
};

struct tfa_cal {
	int rdc;
	int temp;
};

/* ---------------------------------------------------------------------- */

static struct device *tfa_cal_dev;
static int cur_status;
static struct tfa_cal cal_data[MAX_HANDLES];

/* ---------------------------------------------------------------------- */

static ssize_t update_rdc_status(int idx, char *buf)
{
	struct tfa_device *tfa = NULL;
	int ret = 0;
	uint16_t value;
	int size;
	char cal_result[FILESIZE_CAL] = {0};

	tfa = tfa98xx_get_tfa_device_from_index(0);
	if (tfa == NULL)
		return -EINVAL; /* unused device */

	if (idx < 0 || idx >= tfa->dev_count)
		return -EINVAL;

	if (cal_data[idx].rdc == 0
		|| cal_data[idx].rdc == 0xffff) {
		ret = tfa_get_cal_data(idx, &value);
		if (ret == TFA98XX_ERROR_NOT_OPEN)
			value = 0xffff; /* unused device */
		if (ret) {
			pr_info("%s: tfa_cal failed to read data from amplifier\n",
				__func__);
			value = 0;
		}
		if (value == 0xffff)
			pr_info("%s: tfa_cal read wrong data from amplifier\n",
				__func__);
		cal_data[idx].rdc = value;
	}

	snprintf(cal_result, FILESIZE_CAL,
		"%d", cal_data[idx].rdc);

	if (cal_result[0] == 0)
		size = snprintf(buf, 7 + 1, "no_data");
	else
		size = snprintf(buf, strlen(cal_result) + 1,
			"%s", cal_result);

	if (size <= 0) {
		pr_err("%s: tfa_cal failed to show in sysfs file\n", __func__);
		return -EINVAL;
	}

	return size;
}

static ssize_t update_temp_status(int idx, char *buf)
{
	struct tfa_device *tfa = NULL;
	int ret = 0;
	uint16_t value;
	int size;
	char cal_result[FILESIZE_CAL] = {0};

	tfa = tfa98xx_get_tfa_device_from_index(0);
	if (tfa == NULL)
		return -EINVAL; /* unused device */

	if (idx < 0 || idx >= tfa->dev_count)
		return -EINVAL;

	if (cal_data[idx].temp == 0
		|| cal_data[idx].temp == 0xffff) {
		ret = tfa_get_cal_temp(idx, &value);
		if (ret == TFA98XX_ERROR_NOT_OPEN)
			value = 0xffff; /* unused device */
		if (ret) {
			pr_info("%s: tfa_cal failed to read temp from amplifier\n",
				__func__);
			value = 0;
		}
		if (value == 0xffff)
			pr_info("%s: tfa_cal read wrong temp from amplifier\n",
				__func__);
		cal_data[idx].temp = value;
	}

	snprintf(cal_result, FILESIZE_CAL,
		"%d", cal_data[idx].temp);

	if (cal_result[0] == 0)
		size = snprintf(buf, 7 + 1, "no_data");
	else
		size = snprintf(buf, strlen(cal_result) + 1,
			"%s", cal_result);

	if (size <= 0) {
		pr_err("%s: tfa_cal failed to show in sysfs file\n", __func__);
		return -EINVAL;
	}

	return size;
}

static ssize_t SPK1_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int idx = tfa_get_dev_idx_from_inchannel(0);
	int ret;

	ret = update_rdc_status(idx, buf);
	if (ret > 0)
		pr_info("%s: tfa_cal - dev %d - calibration data (rdc %d)\n",
			__func__, idx, cal_data[idx].rdc);
	else
		pr_err("%s: tfa_cal dev %d - error %d\n",
			__func__, idx, ret);

	return ret;
}

static ssize_t SPK1_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	int idx = tfa_get_dev_idx_from_inchannel(0);
	int ret = 0, value = 0;

	ret = kstrtou32(buf, 10, &value);
	if (ret < 0) {
		pr_info("%s: wrong value: %s\n", __func__, buf);
		return -EINVAL;
	}

	ret = tfa_set_cal_data(idx, value);
	if (!ret) {
		cal_data[idx].rdc = value;
		pr_info("%s: tfa_cal - dev %d - calibration data (rdc %d)\n",
			__func__, idx, value);
	}

	return size;
}

static ssize_t SPK2_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int idx = tfa_get_dev_idx_from_inchannel(1);
	int ret;

	ret = update_rdc_status(idx, buf);
	if (ret > 0)
		pr_info("%s: tfa_cal - dev %d - calibration data (rdc %d)\n",
			__func__, idx, cal_data[idx].rdc);
	else
		pr_err("%s: tfa_cal dev %d - error %d\n",
			__func__, idx, ret);

	return ret;
}

static ssize_t SPK2_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	int idx = tfa_get_dev_idx_from_inchannel(1);
	int ret = 0, value = 0;

	ret = kstrtou32(buf, 10, &value);
	if (ret < 0) {
		pr_info("%s: wrong value: %s\n", __func__, buf);
		return -EINVAL;
	}

	ret = tfa_set_cal_data(idx, value);
	if (!ret) {
		cal_data[idx].rdc = value;
		pr_info("%s: tfa_cal - dev %d - calibration data (rdc %d)\n",
			__func__, idx, value);
	}

	return size;
}

static ssize_t SPK3_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int idx = tfa_get_dev_idx_from_inchannel(2);
	int ret;

	ret = update_rdc_status(idx, buf);
	if (ret > 0)
		pr_info("%s: tfa_cal - dev %d - calibration data (rdc %d)\n",
			__func__, idx, cal_data[idx].rdc);
	else
		pr_err("%s: tfa_cal dev %d - error %d\n",
			__func__, idx, ret);

	return ret;
}

static ssize_t SPK3_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	int idx = tfa_get_dev_idx_from_inchannel(2);
	int ret = 0, value = 0;

	ret = kstrtou32(buf, 10, &value);
	if (ret < 0) {
		pr_info("%s: wrong value: %s\n", __func__, buf);
		return -EINVAL;
	}

	ret = tfa_set_cal_data(idx, value);
	if (!ret) {
		cal_data[idx].rdc = value;
		pr_info("%s: tfa_cal - dev %d - calibration data (rdc %d)\n",
			__func__, idx, value);
	}

	return size;
}

static ssize_t SPK4_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int idx = tfa_get_dev_idx_from_inchannel(3);
	int ret;

	ret = update_rdc_status(idx, buf);
	if (ret > 0)
		pr_info("%s: tfa_cal - dev %d - calibration data (rdc %d)\n",
			__func__, idx, cal_data[idx].rdc);
	else
		pr_err("%s: tfa_cal dev %d - error %d\n",
			__func__, idx, ret);

	return ret;
}

static ssize_t SPK4_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	int idx = tfa_get_dev_idx_from_inchannel(3);
	int ret = 0, value = 0;

	ret = kstrtou32(buf, 10, &value);
	if (ret < 0) {
		pr_info("%s: wrong value: %s\n", __func__, buf);
		return -EINVAL;
	}

	ret = tfa_set_cal_data(idx, value);
	if (!ret) {
		cal_data[idx].rdc = value;
		pr_info("%s: tfa_cal - dev %d - calibration data (rdc %d)\n",
			__func__, idx, value);
	}

	return size;
}

static ssize_t temp0_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int idx = tfa_get_dev_idx_from_inchannel(0);
	int ret;

	ret = update_temp_status(idx, buf);
	if (ret > 0)
		pr_info("%s: tfa_cal - dev %d - calibration data (temp %d)\n",
			__func__, idx, cal_data[idx].temp);
	else
		pr_err("%s: tfa_cal dev %d - error %d\n",
			__func__, idx, ret);

	return ret;
}

static ssize_t temp0_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	pr_info("%s: dev %d - not allowed to write temperature in calibration\n",
		__func__, tfa_get_dev_idx_from_inchannel(0));

	return size;
}

static ssize_t temp1_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int idx = tfa_get_dev_idx_from_inchannel(1);
	int ret;

	ret = update_temp_status(idx, buf);
	if (ret > 0)
		pr_info("%s: tfa_cal - dev %d - calibration data (temp %d)\n",
			__func__, idx, cal_data[idx].temp);
	else
		pr_err("%s: tfa_cal dev %d - error %d\n",
			__func__, idx, ret);

	return ret;
}

static ssize_t temp1_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	pr_info("%s: dev %d - not allowed to write temperature in calibration\n",
		__func__, tfa_get_dev_idx_from_inchannel(1));

	return size;
}

static ssize_t temp2_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int idx = tfa_get_dev_idx_from_inchannel(2);
	int ret;

	ret = update_temp_status(idx, buf);
	if (ret > 0)
		pr_info("%s: tfa_cal - dev %d - calibration data (temp %d)\n",
			__func__, idx, cal_data[idx].temp);
	else
		pr_err("%s: tfa_cal dev %d - error %d\n",
			__func__, idx, ret);

	return ret;
}

static ssize_t temp2_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	pr_info("%s: dev %d - not allowed to write temperature in calibration\n",
		__func__, tfa_get_dev_idx_from_inchannel(2));

	return size;
}

static ssize_t temp3_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int idx = tfa_get_dev_idx_from_inchannel(3);
	int ret;

	ret = update_temp_status(idx, buf);
	if (ret > 0)
		pr_info("%s: tfa_cal - dev %d - calibration data (temp %d)\n",
			__func__, idx, cal_data[idx].temp);
	else
		pr_err("%s: tfa_cal dev %d - error %d\n",
			__func__, idx, ret);

	return ret;
}

static ssize_t temp3_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	pr_info("%s: dev %d - not allowed to write temperature in calibration\n",
		__func__, tfa_get_dev_idx_from_inchannel(3));

	return size;
}

static ssize_t status_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int size;
	char cal_state[FILESIZE_CAL] = {0};

	snprintf(cal_state, FILESIZE_CAL,
		"%s", cur_status ?
		"enabled" /* calibration is active */
		: "disabled"); /* calibration is inactive */

	size = snprintf(buf, strlen(cal_state) + 1,
		"%s", cal_state);

	return size;
}

static ssize_t status_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct tfa_device* tfa = NULL;
	int idx, ndev = MAX_HANDLES;
	uint16_t value;
	int ret = 0, status;
	int ret0 = 0, ret1 = 0;

	/* Compare string, excluding the trailing \0 and the potentials eol */
	if (!sysfs_streq(buf, "1") && !sysfs_streq(buf, "0")) {
		pr_info("%s: tfa_cal invalid value to start calibration\n",
			__func__);
		return -EINVAL;
	}

	ret = kstrtou32(buf, 10, &status);
	if (ret < 0) {
		pr_info("%s: wrong value: %s\n", __func__, buf);
		return -EINVAL;
	}
	if (!status) {
		pr_info("%s: do nothing\n", __func__);
		return -EINVAL;
	}
	if (cur_status) {
		pr_info("%s: tfa_cal prior calibration still runs\n", __func__);
		return -EINVAL;
	}

	pr_info("%s: tfa_cal begin\n", __func__);
	cur_status = status; /* run - changed to active */

	memset(cal_data, 0, sizeof(struct tfa_cal) * MAX_HANDLES);

	tfa = tfa98xx_get_tfa_device_from_index(0);
	if (tfa == NULL || tfa->dev_count < 1)
		return -EINVAL; /* unused device */

	ndev = tfa->dev_count;
	for (idx = 0; idx < ndev; idx++) {
		tfa = tfa98xx_get_tfa_device_from_index(idx);
		if (tfa && tfa->func == 0) {
			/* run calibration */
			ret0 = tfa_run_cal(0); /* tfadsp0 */
			break;
		}
	}
	for (idx = 0; idx < ndev; idx++) {
		tfa = tfa98xx_get_tfa_device_from_index(idx);
		if (tfa && tfa->func == 1) {
			/* run calibration */
			ret1 = tfa_run_cal(1); /* tfadsp1 */
			break;
		}
	}
	cur_status = 0; /* done - changed to inactive */

	if (ret0 || ret1) {
		pr_err("%s: tfa_cal failed, ret0 %d, ret1 %d\n", 
			__func__, ret0, ret1);
	}

	for (idx = 0; idx < ndev; idx++) {
		ret = tfa_get_cal_data(idx, &value);
		if (ret || value == 0 || value == 0xffff) {
			/* roll-back to default */
			tfa = tfa98xx_get_tfa_device_from_index(idx);
			tfa->mohm[0] = DUMMY_CALIBRATION_DATA;
			tfa->mtpex = 1;
			cal_data[idx].rdc = DUMMY_CALIBRATION_DATA;
			cal_data[idx].temp = DEFAULT_REF_TEMP;
			tfa->spkr_damaged = 0;
			continue;
		}
		cal_data[idx].rdc = value;
		tfa_get_cal_temp(idx, &value);
		cal_data[idx].temp = value;
		pr_info("%s: tfa%d rdc %d, temp %d\n", __func__, idx,
			cal_data[idx].rdc, cal_data[idx].temp);
	}

	pr_info("%s: tfa_cal end\n", __func__);
	return size;
}

static ssize_t ref_temp_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	u16 temp_val = DEFAULT_REF_TEMP;
	char cal_result[FILESIZE_CAL] = {0};
	int size;
	enum tfa98xx_error ret;

	/* EXT_TEMP */
	ret = tfa98xx_read_reference_temp(&temp_val);
	if (ret)
		pr_err("error in reading reference temp\n");

	snprintf(cal_result, FILESIZE_CAL,
		"%d", temp_val);

	if (cal_result[0] == 0)
		size = snprintf(buf, 7 + 1, "no_data");
	else
		size = snprintf(buf, strlen(cal_result) + 1,
			"%s", cal_result);

	if (size > 0)
		pr_info("%s: tfa_cal - ref_temp %d for calibration\n",
			__func__, temp_val);
	else
		pr_err("%s: tfa_cal - ref_temp error\n",
			__func__);

	return size;
}

static ssize_t ref_temp_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	pr_info("%s: not allowed to write temperature for calibration\n",
		__func__);

	return size;
}

static int reinit_count = 0;
static ssize_t reinit_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct tfa_device *tfa = NULL;
	int count = 0;

	tfa = tfa98xx_get_tfa_device_from_index(0);
	if (!tfa)
		return -ENODEV;
	if (tfa->tfa_family == 0) {
		pr_err("[0x%x] %s: system is not initialized: not probed yet!\n",
			tfa->resp_address, __func__);
		return -EIO;
	}

	pr_debug("[0x%x] reinit : reinit_count %d\n",
		tfa->resp_address, reinit_count);
	count = snprintf(buf, PAGE_SIZE, "reinit requested: %d\n",
		reinit_count);

	return count;
}

static ssize_t reinit_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct tfa_device *tfa = NULL;
	int re_init = 0;

	tfa = tfa98xx_get_tfa_device_from_index(0);
	if (!tfa)
		return -ENODEV;
	if (tfa->tfa_family == 0) {
		pr_err("[0x%x] %s: system is not initialized: not probed yet!\n",
			tfa->resp_address, __func__);
		return -EIO;
	}

	/* check string length, and account for eol */
	if (count < 1)
		return -EINVAL;

	if (!strncmp(buf, "1", 1))
		re_init = 1;
	else if (!strncmp(buf, "0", 1))
		re_init = 0;
	else {
		pr_info("%s: reinit is triggered with %s!\n", __func__, buf);
		return -EINVAL;
	}

	pr_info("%s: reinit < %d\n", __func__, re_init);

	if (re_init) {
		pr_info("%s: started reloading / reinitializing (counter %d)\n",
			__func__, reinit_count + 1);
		tfa98xx_reinit();
		reinit_count++;
	}

	return count;
}

int tfa98xx_cal_init(struct class *tfa_class)
{
	int ret = 0;

	if (tfa_class) {
		tfa_cal_dev = device_create(tfa_class,
			NULL, DEV_ID_TFA_CAL, NULL, TFA_CAL_DEV_NAME);
		if (!IS_ERR(tfa_cal_dev)) {
			ret = sysfs_create_group(&tfa_cal_dev->kobj,
				&tfa_cal_attr_grp);
			if (ret)
				pr_err("%s: failed to create sysfs group. ret (%d)\n",
					__func__, ret);
		}
	}

	pr_info("%s: initialized (%d)\n", __func__,
		(tfa_class != NULL) ? 1 : 0);

	return ret;
}

void tfa98xx_cal_exit(struct class *tfa_class)
{
	device_destroy(tfa_class, DEV_ID_TFA_CAL);
	pr_info("exited\n");
}
