// SPDX-License-Identifier: GPL-2.0

#include <stdlib.h>
#include <string.h>

#include <gem/aes.h>
#include <gem/aes-area.h>
#include <gem/aes-filter.h>
#include <gem/aes-pixel.h>
#include <gem/aes-shape.h>
#include <gem/aes-simple.h>
#include <gem/vdi_.h>

#include "internal/assert.h"

aes_id_t aes_appl_init(struct aes *aes_)
{
	vdi_id_t vdi_id = vdi_v_opnwk(NULL, NULL);

	*aes_ = (struct aes) { .vdi_id = vdi_id };

	return (aes_id_t) { .aes_ = aes_ };
}

void aes_appl_exit(aes_id_t aes_id)
{
	vdi_v_clswk(aes_id.aes_->vdi_id);
}

bool aes_palette_color(aes_id_t aes_id,
	const int index, struct vdi_color *color)
{
	return vq_color(aes_id.aes_->vdi_id, index, color);
}

static bool shape_layer_subdivision(
	const struct aes_area clip,
	struct aes_object_shape_layer *layers,
	const aes_object_shape_layer_f f, void *arg)
{
	const struct aes_area c =
		aes_area_intersection(clip, layers->shape.area);

	if (!aes_area_degenerate(c) && !f(c, layers, arg))
		return false;

	if (!layers->next)
		return true;

	struct aes_area s;

	aes_for_each_area_subdivision (&s, clip, layers->shape.area)
		if (!shape_layer_subdivision(s, layers->next, f, arg))
			return false;

	return true;
}

static bool shape_layers(
	const struct aes_area clip,
	struct aes_object_shape_layer *next,
	struct aes_object_shape_iterator *iterator,
	const aes_object_shape_layer_f f, void *arg)
{
	struct aes_object_shape_layer layer_arg = { .next = next };

	if (!next) {
		if (!iterator->first(&layer_arg.shape, iterator))
			return true;

		return shape_layers(clip, &layer_arg, iterator, f, arg);
	}

	if (!iterator->next(&layer_arg.shape, iterator))
		return shape_layer_subdivision(clip, next, f, arg);

	return shape_layers(clip, &layer_arg, iterator, f, arg);
}

static bool clip_filter(struct aes_object_shape *shape, void *arg)
{
	const struct aes_area *clip = arg;

	return aes_area_overlap(shape->area, *clip);
}

bool aes_object_shape_layers(struct aes_area clip,
	struct aes_object_shape_iterator *iterator,
	const aes_object_shape_layer_f f, void *arg)
{
	struct aes_object_simple_shape_iterator_arg simple_arg;
	struct aes_object_filter_shape_iterator_arg filter_arg;

	struct aes_object_shape_iterator simple_iterator =
		aes_object_simple_shape_iterator(iterator, &simple_arg);
	struct aes_object_shape_iterator clip_iterator =
		aes_object_filter_shape_iterator(clip_filter, &clip,
			&simple_iterator, &filter_arg);

	return shape_layers(clip, NULL, &clip_iterator, f, arg);
}
